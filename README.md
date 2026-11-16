# memalloc — Custom Dynamic Memory Allocator

`memalloc` es un asignador de memoria dinámico (`malloc`, `free`, `calloc`, `realloc`) implementado desde cero en **C11** sobre **GNU/Linux**, administrando directamente el segmento *heap* del proceso mediante llamadas al sistema POSIX (`sbrk`).

El proyecto explora la gestión de memoria virtual en espacio de usuario, aritmética de punteros, alineación estricta de arquitectura (`x86_64`) y estrategias clásicas para mitigar tanto la fragmentación interna como la externa.

## Características

* **`myMalloc(size_t size)`**: Asignación dinámica alineada a 16 bytes con reciclaje de bloques libres mediante búsqueda **First-Fit** y división de bloques sobrantes (**Splitting**).
* **`myFree(void *ptr)`**: Liberación en $O(1)$ con fusión automática de bloques libres contiguos (**Coalescing** bidireccional) para prevenir la fragmentación externa.
* **`myCalloc(size_t nmemb, size_t size)`**: Asignación e inicialización a cero (`memset`) con protección contra desbordamiento entero (*Integer Overflow* frente a `SIZE_MAX`).
* **`myRealloc(void *ptr, size_t size)`**: Redimensionamiento inteligente con reducción *in-place*, expansión *in-place* absorbiendo bloques vecinos libres sin copiar datos, y relocalización segura cuando el crecimiento contiguo no es posible.

## Diseño Interno

Cada región reservada en el *heap* consta de un encabezado oculto de metadatos (`BlockHeader`) de **32 bytes** alineado mediante `alignas(16)`, seguido inmediatamente por el área útil devuelta al usuario:

```text
+---------------------------------------+-----------------------------------+
|        BlockHeader (32 bytes)         |     Área de datos del usuario     |
|  - alignas(16) size_t size            |     <-- ptr devuelto al caller    |
|  - bool isFree                        |     (múltiplo exacto de 16 bytes)  |
|  - struct BlockHeader *next           |                                   |
+---------------------------------------+-----------------------------------+
^                                       ^
sbrk() / lista enlazada (head..tail)    (void *)(block + 1)
```

### Decisiones técnicas clave
* **Alineación ABI (`x86_64`)**: Tanto el tamaño de `BlockHeader` (32 bytes) como el tamaño solicitado por el usuario (`(size + 15) & ~((size_t)15)`) están alineados a 16 bytes, garantizando compatibilidad con instrucciones vectoriales (SSE/AVX) y evitando fallos detectados por `UndefinedBehaviorSanitizer`.
* **Inserción en $O(1)$ al final del heap**: Mantenimiento simultáneo de punteros `head` y `tail` sobre la lista enlazada de bloques para evitar recorridos innecesarios al expandir el *program break*.
* **Umbral mínimo de división (*Splitting*)**: Un bloque libre solo se divide si el espacio sobrante permite alojar un nuevo `BlockHeader` (32 bytes) más un bloque mínimo útil (16 bytes), evitando crear fragmentos inutilizables.

## Estructura del Proyecto

```text
memalloc/
├── include/
│   └── memalloc.h    # Interfaz pública (myMalloc, myFree, myCalloc, myRealloc)
├── src/
│   └── memalloc.c    # Lógica de metadatos, sbrk, First-Fit, Splitting y Coalescing
├── tests/
│   └── test_main.c   # Suite de pruebas unitarias y verificación de alineación
└── Makefile          # Compilación estricta con -fsanitize=undefined
```

## Compilación y Ejecución

### Requisitos
* Compilador compatible con **C11** (`gcc` o `clang`)
* `make`
* Entorno Linux / POSIX

### Construir y correr los tests
```bash
make run
```

### Limpiar artefactos compilados
```bash
make clean
```

## Posibles Extensiones Futuras
* Soporte para asignaciones grandes mediante `mmap()` y `munmap()` (a partir de un umbral como 128 KB) para devolver memoria física directamente al sistema operativo.
* Lista doblemente enlazada explícita de bloques libres (*Explicit Free List*) para acelerar la búsqueda de $O(N)$ bloques totales a $O(K)$ bloques libres.
* Seguridad en entornos multihilo (*Thread-safety*) mediante `pthread_mutex_t` o arenas por hilo.
