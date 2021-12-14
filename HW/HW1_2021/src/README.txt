Files required: cache_model.h cache_model.c main.c
Command Prompt commands for compiling:
1.-(a) cache enable: gcc -DPROBLEM='A' -DENABLE=1 main.c cache_model.c
1.-(a) cache disable: gcc -DPROBLEM='A' -DENABLE=0 main.c cache_model.c
1.-(b) cache enable: gcc -DPROBLEM='B' -DENABLE=1 main.c cache_model.c
1.-(b) cache disable: gcc -DPROBLEM='B' -DENABLE=0 main.c cache_model.c