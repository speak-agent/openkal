/* openkal.exec --- memory a program may execute.
 *
 * Optional. The interface exists because a program above openkal otherwise has
 * no way to obtain such memory at all, and several ordinary kinds of program
 * need it: an interpreter that compiles, a pattern matcher with a compiled
 * mode, a shader compiler, a loader of any format. None of them can be written
 * above openkal without it, and none of them is unusual.
 *
 * It is deliberately not an operation that loads a program. Loading is parsing
 * a format, relocating, and binding names, and the answer to each is the same
 * in every environment --- so by clause 7.1's reasoning a loader belongs above
 * openkal, written once, and what it needs from beneath is only this.
 *
 * The interface is three operations rather than one because every environment
 * this specification targets now distinguishes writing from executing, and an
 * interface that returned memory which was both would be unimplementable on
 * two of them. A region is writable, then published, then executable; it is
 * never both.
 */
#ifndef OPENKAL_EXEC_H
#define OPENKAL_EXEC_H
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Reserves a region of at least size bytes that may be written and, after
 * kal_exec_publish, executed. Returns a null pointer if the region cannot be
 * reserved.
 *
 * The region is not executable when it is returned. A caller that executes it
 * without publishing it has undefined behaviour, and every environment this
 * specification targets will fault rather than proceed.
 *
 * Alignment is the environment's page granularity or coarser; a caller that
 * needs a particular alignment obtains it within the region. There is no
 * alignment parameter because the granularity that matters here is the one the
 * environment protects at, and a caller cannot usefully ask for less. */
void* kal_exec_alloc(kal_uintptr size);

/* Makes a region obtained from kal_exec_alloc executable and no longer
 * writable. Returns kal_ok, or an error if the environment refuses.
 *
 * The whole of the region is published. An implementation that had to publish
 * in parts would be reporting its page granularity through the shape of the
 * interface, and a caller that generates code cannot act upon that. The caller
 * that can --- a loader --- has kal_exec_publish_part, below. */
int kal_exec_publish(void* p, kal_uintptr size);

/* Releases a region obtained from kal_exec_alloc, published or not. The size is
 * the one passed to the reservation. */
void kal_exec_free(void* p, kal_uintptr size);

/* THE PARTIAL FORM, ADDED IN 0.15, FOR THE ONE CALLER THAT CAN ACT UPON A
 * GRANULARITY.
 *
 * kal_exec_publish takes the whole region because a program that generates
 * code has no use for the environment's page size. A loader has, and cannot
 * work without a partial form: the object it loads was linked with its
 * instructions and its data at fixed distances from each other, so both lie in
 * one region, and the instructions must become executable while the data stays
 * writable. Two regions would not do --- their distance is the environment's
 * choice, and the object's code reaches its data by that distance. The object
 * states the alignment of each of its parts, so the loader compares it with
 * the granularity below before it reserves anything, and refuses an object
 * whose parts are aligned more finely than the environment protects, with a
 * reason, rather than publishing too much or too little of it. */

/* The quantum in which part of a region is published: an offset and a size that
 * are multiples of it are accepted by kal_exec_publish_part in every
 * environment. It is the granularity at which the environment protects memory,
 * which can be finer than the one kal_memory_granularity reports --- that one
 * is also the granularity of allocation, and on one system the two differ. */
kal_uintptr kal_exec_granularity(void);

/* Publishes part of a region obtained from kal_exec_alloc: the size bytes from
 * offset become executable and no longer writable, and the rest of the region
 * is left as it was. Returns kal_ok; kal_err_invalid, with nothing changed, if
 * the region is null, size is zero, or offset or size is not a multiple of
 * kal_exec_granularity(); or an error if the environment refuses. The part
 * shall lie within the region.
 *
 * A region may have several parts published, and a part may be published
 * again where KAL_EXEC_PROP_REPUBLISH is set. kal_exec_free releases the whole
 * region, whichever of its parts were published. The instruction cache is the
 * program's to maintain, as it is after kal_exec_publish. */
int kal_exec_publish_part(void* p, kal_uintptr offset, kal_uintptr size);

/* Positions in kal_exec_props. */

/* A published region may be reserved again for writing --- kal_exec_alloc
 * followed by kal_exec_publish is not the only order the implementation
 * supports, and a caller may write to a region it has already published by
 * publishing it again. Where the position is zero, a caller that must change
 * published bytes reserves a second region and abandons the first. */
#define KAL_EXEC_PROP_REPUBLISH ((kal_uintptr)1u << 0)

/* Whether memory this program may execute is available to THIS ARTIFACT in THIS
 * environment.
 *
 * CLAUSE 6.5 SETTLED THIS AT DEPENDENCY RESOLUTION, AND THAT ANSWER DOES
 * NOT SURVIVE AN ARTIFACT THAT IS DISTRIBUTED. One system grants such memory
 * only to a program carrying a signed declaration, applied after the link by
 * whoever produces the artifact --- so when the artifact is built for its own
 * machine, the party that decides is the party that resolves the dependency and
 * 6.5 is right. When the artifact is produced once and run in many
 * environments, that party decided for all of them, possibly years earlier, and
 * the consumer resolves nothing.
 *
 * ⇒ The interface is provided either way; whether it can be exercised is read
 * here. This is not the shape clause 6.2 forbids --- an operation present and
 * always failing is a defect BECAUSE THE CALLER CANNOT TELL, and a position the
 * caller reads first is exactly what tells it.
 *
 * 6.5's own objection stands and is answered elsewhere: a path few artifacts
 * take is a path little verified. The conformance arrangement answers this
 * position both ways, which is the only way the unavailable path is exercised
 * at all. */
#define KAL_EXEC_PROP_AVAILABLE ((kal_uintptr)1u << 1)

kal_uintptr kal_exec_props(void);

#ifdef __cplusplus
}
#endif
#endif /* OPENKAL_EXEC_H */
