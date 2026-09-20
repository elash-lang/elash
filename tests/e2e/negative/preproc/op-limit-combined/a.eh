#func foo(i)
    #if i >= 0
        #return foo(i - 1)
    #end
    #return 0
#end

#const bar = foo(60)
#include b
