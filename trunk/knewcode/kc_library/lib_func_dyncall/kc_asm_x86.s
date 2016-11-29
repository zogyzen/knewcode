.text
.globl ___DybCallFunc
.def ___DybCallFunc;	.scl	2;	.type	32;	.endef

#��ຯ��������̬���ú���
 ___DybCallFunc:
    push       %ebp
    mov        %esp, %ebp
#����1�� 8(%ebp)    thisָ��
#����2��12(%ebp)    ����ָ��
#����3��16(%ebp)    �Ƿ��з���ֵ��δ�ã�
#����4��20(%ebp)    ����ֵ���ͣ�δ�ã�
#����5��24(%ebp)    ��������
#����6��28(%ebp)    ����ջ�ߴ�
#����7��32(%ebp)    ������������
#����8��36(%ebp)    ����ֵ����

#��ʼ
 ___start:
    sub 28(%ebp), %esp;                             #�������ջ
    mov $0, %edx;                                   #��ʼ������ջƫ��ֵ
    mov $0, %ecx;                                   #��ʼ��ѭ��������

#��ʼѭ����Ӳ���
 ___start_loop:
    cmp 24(%ebp), %ecx;                             #�ж�ѭ���������Ƿ񳬳���������
    jge ___exit_loop_to_call_func;                  #���ѭ���������Ƿ�Ϊ0��������ѭ��
    mov 32(%ebp), %eax;                             #�õ��������͵�ַ
    mov (%eax,%ecx,4), %eax;                        #�õ���������ֵ

#��������
 ___bool_type:
    cmp $1, %eax;                                   #�жϲ����Ƿ񲼶�����
    jne ___int_type;                                #��������ת����һ��������
    mov 36(%ebp), %eax;                             #�õ�����ֵ�������ַ
    mov (%eax,%ecx,4), %eax;                        #��ȡ����ֵ��ַ
    mov (%eax), %al;                                #��ȡ�����Ͳ���ֵ
    movzbl %al, %eax;                               #�����Ͳ���ֵת��Ϊ4�ֽ�
    mov %eax, (%esp,%edx,1);                        #����ֵ��ջ
    add $4, %edx;                                   #����ջƫ��
    jmp ___next_loop;                               #�´�ѭ��

#����
 ___int_type:
    cmp $2, %eax;                                   #�жϲ����Ƿ�����
    jne ___double_type;                             #��������ת����һ��������
    mov 36(%ebp), %eax;                             #�õ�����ֵ�������ַ
    mov (%eax,%ecx,4), %eax;                        #��ȡ����ֵ��ַ
    mov (%eax), %eax;                               #��ȡ���Ͳ���ֵ
    mov %eax, (%esp,%edx,1);                        #����ֵ��ջ
    add $4, %edx;                                   #����ջƫ��
    jmp ___next_loop;                               #�´�ѭ��

#˫���ȸ�������
 ___double_type:
    cmp $3, %eax;                                   #�жϲ����Ƿ�Ϊ˫���ȸ�������
    jne ___pointer_type;                            #��������ת����һ��������
    mov 36(%ebp), %eax;                             #�õ�����ֵ�������ַ
    mov (%eax,%ecx,4), %eax;                        #��ȡ����ֵ��ַ
    fldl (%eax);                                    #��ȡ�����Ͳ���ֵ
    fstpl (%esp,%edx,1);                            #�����Ͳ���ֵ��ջ
    add $8, %edx;                                   #����ջƫ��
    jmp ___next_loop;                               #�´�ѭ��

#�����Ϊָ������
 ___pointer_type:
    mov 36(%ebp), %eax;                             #�õ�����ֵ�������ַ
    mov (%eax,%ecx,4), %eax;                        #��ȡ����ֵ��ַ
    mov (%eax), %eax;                               #��ȡָ���Ͳ���ֵ
    mov %eax, (%esp,%edx,1);                        #����ֵ��ջ
    add $4, %edx;                                   #����ջƫ��

#����ѭ����������ʼ�´�ѭ��
 ___next_loop:
    inc %ecx;                                       #ѭ����������1
    jmp ___start_loop;                              #��ͷ��ʼѭ��

#����ѭ������ʼ���ú���
 ___exit_loop_to_call_func:
    mov 8(%ebp), %ecx;                              #���thisָ��
    cmp $0, %ecx;                                   #�ж�thisָ���Ƿ�Ϊ��
    je ___begin_call_func;                          #���thisָ�벻Ϊ�գ��������Ա�������轫thisָ��ѹջ��cdecl����Լ����
    push %ecx;                                      #ѹջthisָ�루cdecl����Լ����
 ___begin_call_func:
    call 12(%ebp);                                  #���ú���

#��ɺ�������
    mov 8(%ebp), %ecx;                              #�����ж�thisָ���Ƿ�Ϊ��
    cmp $0, %ecx;                                   #�ж�thisָ���Ƿ�Ϊ��
    je ___finish;                                   #���thisָ�벻Ϊ�գ��������Ա�������轫thisָ�뵯ջ��cdecl����Լ����
    pop %ecx;                                       #��ջthisָ�루cdecl����Լ����
 ___finish:
    add 28(%ebp), %esp;                             #���thisָ��Ϊ�գ�����ȫ�ֺ�����Ĭ�ϲ���cdecl�ĵ���Լ�����ɵ����߸����������ջ

#��ຯ������
    mov        %ebp, %esp
    pop        %ebp
    ret
