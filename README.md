## Dynamic thread-safe allocator like as 'malloc'
```console
/***************************************
 * Dynamic allocation like as 'malloc' *
 * Author: Acool4ik                    *
 *                                     *
 *      Params:			               *
 * Thread-safety		               *
 * Min allocated mem: 4Byte            *
 * Max allocated mem: 8Gib             *
 *                                     *
 *      Complexity (API)               *
 * mysetup: O(1)                       *
 * myteardown: O(1)                    *
 * myalloc: O(n)                       *
 * myfree: O(1)                        *
 * print_all_fragments: O(n)           *
 *                                     *
 *      Algorithm                      *
 * Border markers                      *
 *                                     *
 ***************************************/
```
### Then to run
```make && make run```
```make && make run_with_valgring```
### Memory after allocate
```console
start fragment [94718164445840]
	[1][94718164445840]	{blocks: 3, isFree: 0}
	[2][94718164445844]	{data: 0}
	[3][94718164445848]	{blocks: 3, isFree: 0}
end fragment [94718164445840]
start fragment [94718164445852]
	[1][94718164445852]	{blocks: 4, isFree: 0}
	[2][94718164445856]	{data: 0}
	[3][94718164445860]	{data: 0}
	[4][94718164445864]	{blocks: 4, isFree: 0}
end fragment [94718164445852]
start fragment [94718164445868]
	[1][94718164445868]	{blocks: 5, isFree: 0}
	[2][94718164445872]	{data: 0}
	[3][94718164445876]	{data: 0}
	[4][94718164445880]	{data: 0}
	[5][94718164445884]	{blocks: 5, isFree: 0}
end fragment [94718164445868]
```
### Memory after free
```console
start fragment [94718164445840]
	[1][94718164445840]	{blocks: 12, isFree: 1}
	[2][94718164445844]	{data: 0}
	[3][94718164445848]	{data: -2147483645}
	[4][94718164445852]	{data: -2147483644}
	[5][94718164445856]	{data: 0}
	[6][94718164445860]	{data: 0}
	[7][94718164445864]	{data: -2147483641}
	[8][94718164445868]	{data: -2147483643}
	[9][94718164445872]	{data: 0}
	[10][94718164445876]	{data: 0}
	[11][94718164445880]	{data: 0}
	[12][94718164445884]	{blocks: 12, isFree: 1}
end fragment [94718164445840]
```
