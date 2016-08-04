EC Extend Cache.

Partial writes on an ec pool will have a Two Phase Commit (TPC) scheme. In TPC initial “prepare” phase, the partial data will be prepared, partial parities will be calculated and write partial data and parity to participating shards as temporary shard objects. Once partial data and parities are written to participating shards, a “roll-forward” queue will be populated with current op (to be "roll forward"d later) and clients will be write acknowledged. Likewise more partial writes will be accumulated which has to be readable after clients is write acknowledged. Without extent cache subsequent reads have to perform multiple reads to shards, coalesce temporary objects, in order and fulfill read request. To reduce number of shard reads to fulfill client reads, partially written data will be cached in primary shard while write "prepare" phase.

Non-"rollforward"d data in cache are non-evict-able. Once extent cache reaches a high threshold of utilization, such event will trigger "roll forward" of eligible objects until a low threshold of cache usage is achieved. Client writes will not be stalled on reaching high threshold, however will be blocked if cache usage reaches full utilization.

Extent cache is designed as follows,

Extent Cache is a set of cache objects, have predefined/configurable size(pool level), have a high and low usage threshold, overall protected by lock.
Each cache object is a set of cache extents, having total cached size, identified uniquely by "hoid", flagged evict-able, reference count and is protected by lock.
Each cache extent have non-overlapping ranges of data of "hoid" object.
Extent cache will not keep versions of partially written objects nor the order which it’s been written, rather will have coalesced version of "prepare"d data.

Cache will be initialized on ECBackend init.
src/osd/ECBackend.h

Cache is implemented in src/osd/ECCache.hpp
gtest code in src/test/osd/TestECCache.cc
Enabled with following config files
src/test/Makefile.am
src/test/osd/CMakeLists.txt

Cache is warmed in write path, "check_op()" is an optimal place. Will flag respective cache object "non-evict-able". A simple reference is held when "copyin"
For test purpose, with current AppendOp, in "generate_transactions()" -> "void operator()(const ECTransaction::AppendOp &op)" cache is populated.

in read path "objects_read_async()", for a given , possible extents will be read from cache and only remaining extents will be read via existing read path. CallClientContexts need to be updated accordingly. A simple reference is held when "copyout"

While cache population, if high usage threshold is surpassed by current Op length, "roll forward" will to be triggered on associated "roll-forward queue".
"roll forward" should be ideally designed to run in a context other than current write context, otherwise the current write will suffer latency.
Triggering of "roll forward" is not tested currently, simply a message is pop'd on reaching high threshold. "roll-forward" processing will flag respective cache object "evict-able"

NOTE on extended use of the extent cache follows,
For calculating parity, need entire stripe data(delta parity use case is an exclusion), however with partial writes it’s possible that only partial stripe is present in incoming op. In such case non-participating shard striplets need to be read in to re-calculate the parity. Extent cache will be used to cache such striplets too. Such will reduce the reads to be performed to fulfill partial writes (sequential partial write use case). Though this is not preliminary purpose if this cache, will use the cache for this purpose too as an added optimization.

