# stalloc

stalloc is an _elementary_ memory manager, based off of
[http://wiki.osdev.org/User:Pancakes/BitmapHeapImplementation]

given a heap, stalloc will be able to track allocated objects and free them.
the user will create blocks (preferably in 2^n sizes) comprising of chunks (also 2^n sizes)
after the user has initialzed their blocks, they may use *stalloc()* and *free()*
