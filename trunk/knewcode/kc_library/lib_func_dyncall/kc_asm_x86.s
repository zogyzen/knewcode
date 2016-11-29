.text
.globl ___DybCallFunc
.def ___DybCallFunc;	.scl	2;	.type	32;	.endef

#汇编函数——动态调用函数
 ___DybCallFunc:
    push       %ebp
    mov        %esp, %ebp
#参数1： 8(%ebp)    this指针
#参数2：12(%ebp)    函数指针
#参数3：16(%ebp)    是否有返回值（未用）
#参数4：20(%ebp)    返回值类型（未用）
#参数5：24(%ebp)    参数数量
#参数6：28(%ebp)    参数栈尺寸
#参数7：32(%ebp)    参数类型数组
#参数8：36(%ebp)    参数值数组

#开始
 ___start:
    sub 28(%ebp), %esp;                             #分配参数栈
    mov $0, %edx;                                   #初始化参数栈偏移值
    mov $0, %ecx;                                   #初始化循环计数器

#开始循环添加参数
 ___start_loop:
    cmp 24(%ebp), %ecx;                             #判断循环计数器是否超出参数数量
    jge ___exit_loop_to_call_func;                  #如果循环计数器是否为0，则跳出循环
    mov 32(%ebp), %eax;                             #得到参数类型地址
    mov (%eax,%ecx,4), %eax;                        #得到参数类型值

#布尔类型
 ___bool_type:
    cmp $1, %eax;                                   #判断参数是否布尔类型
    jne ___int_type;                                #不是则跳转到下一参数类型
    mov 36(%ebp), %eax;                             #得到参数值数组基地址
    mov (%eax,%ecx,4), %eax;                        #获取参数值地址
    mov (%eax), %al;                                #获取布尔型参数值
    movzbl %al, %eax;                               #布尔型参数值转换为4字节
    mov %eax, (%esp,%edx,1);                        #参数值入栈
    add $4, %edx;                                   #调整栈偏移
    jmp ___next_loop;                               #下次循环

#整型
 ___int_type:
    cmp $2, %eax;                                   #判断参数是否整型
    jne ___double_type;                             #不是则跳转到下一参数类型
    mov 36(%ebp), %eax;                             #得到参数值数组基地址
    mov (%eax,%ecx,4), %eax;                        #获取参数值地址
    mov (%eax), %eax;                               #获取整型参数值
    mov %eax, (%esp,%edx,1);                        #参数值入栈
    add $4, %edx;                                   #调整栈偏移
    jmp ___next_loop;                               #下次循环

#双精度浮点类型
 ___double_type:
    cmp $3, %eax;                                   #判断参数是否为双精度浮点类型
    jne ___pointer_type;                            #不是则跳转到下一参数类型
    mov 36(%ebp), %eax;                             #得到参数值数组基地址
    mov (%eax,%ecx,4), %eax;                        #获取参数值地址
    fldl (%eax);                                    #获取浮点型参数值
    fstpl (%esp,%edx,1);                            #浮点型参数值入栈
    add $8, %edx;                                   #调整栈偏移
    jmp ___next_loop;                               #下次循环

#其余的为指针类型
 ___pointer_type:
    mov 36(%ebp), %eax;                             #得到参数值数组基地址
    mov (%eax,%ecx,4), %eax;                        #获取参数值地址
    mov (%eax), %eax;                               #获取指针型参数值
    mov %eax, (%esp,%edx,1);                        #参数值入栈
    add $4, %edx;                                   #调整栈偏移

#本次循环结束，开始下次循环
 ___next_loop:
    inc %ecx;                                       #循环计数器加1
    jmp ___start_loop;                              #从头开始循环

#结束循环，开始调用函数
 ___exit_loop_to_call_func:
    mov 8(%ebp), %ecx;                              #添加this指针
    cmp $0, %ecx;                                   #判断this指针是否为空
    je ___begin_call_func;                          #如果this指针不为空，则是类成员函数，需将this指针压栈（cdecl调用约定）
    push %ecx;                                      #压栈this指针（cdecl调用约定）
 ___begin_call_func:
    call 12(%ebp);                                  #调用函数

#完成函数调用
    mov 8(%ebp), %ecx;                              #用于判断this指针是否为空
    cmp $0, %ecx;                                   #判断this指针是否为空
    je ___finish;                                   #如果this指针不为空，则是类成员函数，需将this指针弹栈（cdecl调用约定）
    pop %ecx;                                       #弹栈this指针（cdecl调用约定）
 ___finish:
    add 28(%ebp), %esp;                             #如果this指针为空，则是全局函数，默认采用cdecl的调用约定，由调用者负责清除参数栈

#汇编函数返回
    mov        %ebp, %esp
    pop        %ebp
    ret
