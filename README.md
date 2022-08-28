<!--
 * @Author: lizhiyuan
 * @Date: 2020-11-21 20:53:10
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-03-14 17:50:06
-->

# 操作系统

## 冯诺依曼存储思想

将程序和数据存放到计算机的内部存储器中,计算机在程序控制下一步一步的进行处理

计算机由五大部件组成:输入设备、输出设备、存储器、运算器、控制器

通俗的解释就是:计算机的工作原理是将程序放到内存中,然后用一个指针指向它(cs:ip),然后取指执行,取指执行.....

形象来看的话:当计算机打开一个 APP 应用的时候,实际是开启了一个进程,该进程会被操作系统自动执行...

总结: 取指 + 执行

## 数据的存储

```
int a = 10;

```

对于无符号数的存储,我们只需要考虑将代码中的 10 转化成二进制即可

```
int a = -1

```

10000 - 0001 = 1111

对于一个有符号数的存储,我们只需要考虑将他正数的使用他的模 - 本身 （二进制取反+1）

```
short int a = -12345 // 有符号转无符号数
unsigned short b = (unsigned short)a;

```

对于有符号数转化为无符号数的话,分为两种情况

- 当有符号的数值 < 0 的时候,存储的二进制位不变,显示变化(代表着最高位为 1),相当于十进制加上 2n 次方

- 当有符号的数值 >= 0 的时候,存储的二进制位不变,显示不变(代表着最高位为 0)

在看一个例子:

```
int a = -1;
unsigned int b = 0;
if(a < b) // 假设是无符号数和有符号数做判断,强制将有符号数转化为无符号数来进行对比
```

结果 1111 < 0000

```
unsigned int a = 10
int b = (int)a
```

对于无符号数转化为有符号数,也分两种情况

- 当无符号的最高位为 0 的时候,二进制位不变,显示不变
- 当无符号的最高位未 1 的时候,二进制位不变,显示变化(等同于该十进制数值 - 2n 次方,最高位被占用了)

无符号数从较小数据类型 --> 较大数据类型,前面补 0 即可
有符号数从较小数据类型 --> 较大数据类型,前面补 1 即可 （补出来的数其实就是-1）

总结: 永远记住,一个负数在编译阶段,就已经以补码的形式存储了,只需要正常的进行加减法即可..

## 操作系统启动简略

硬件上电后,操作系统在硬盘上(也有可能是在光盘中)

我们接下来要做的事儿就是把磁盘上的代码放入内存中,将 cs:ip 的指针指向它,让计算机取指执行

BIOS 会先读取磁盘上的 0 磁道 0 扇区(1 个扇区) ----> 0x07c00 引导扇区 bootsect.s (这时候会将 07c00 的内存空间腾出来,放到 0x90000 处执行....)

0x13 中断继续读磁盘中的第 2-5 个扇区(4 个扇区) ----> 0x90200(bootsect 512 个字节,0x90000 + 512 个字节 = 0x90200) setup.s

setup.s 会将操作系统的代码移动到 0 地址--0x07c00 的内存地址上去

最后 setup.s 会切换到保护模式,32 位(寻址)模式.跳到 0 地址去执行操作系统的代码

**_总结:BIOS ----> bootsect.s ----> setup.s -----> system_**

main 函数开始初始化操作系统,main 函数永不退出,永不返回....

```c
void main(void){
    mem_init();
    trap_init();
    blk_dev_init();
    chr_dev_init();
    tty_init();
    time_init();
    sched_init();
    buffer_init();
    hd_init();
    floppy_init();
    sti();
    move_to_use_mode();
    if(!fork()){init()};
}
```

从此以后,操作系统内核进入后台,成为中断/异常处理程序

## 中断/异常

中断是异步操作....CPU 和硬件设备是可以并行操作的,CPU 不必等待硬件 IO 设备的操作结果,当设备完成的时候,由设备发出中断信号告知 CPU,CPU 立即保存当前的执行情况后处理。。。

这就是操作系统首次实现异步的一个机制: 中断机制

- IO 中断
- 时钟中断 进程轮片/定时
- 硬件故障

异常其实也是种中断,只不过通常异常是 CPU 主动触发的一种中断

- 系统调用
- 页故障/页错误
- 保护性异常 (读写冲突)
- 断点调试
- 其他程序异常

总结: 中断是一些外部事件,被动触发的,异常是由正在执行的指令引起的，属于主动触发的....

## 工作原理

CPU 会在每条指令执行周期的最后时刻扫描中断寄存器,查看是否有中断信号,若有中断,通过查找中断向量表引出中断处理函数

- 128 0x80 系统调用异常中断
- 32 - 127 外部中断,IO 设备中断

举例：IO 设备硬中断

- 打印机给 CPU 发送中断信号
- CPU 处理完当前指令后检测到中断,判断出中断来源并向相关设备发确认信号
- CPU 进行`系统调用`,切换到内核态，并将现场保存(程序计数器 PC 以及程序状态字 PSW)
- CPU 根据中断向量表,获得该中断相关处理程序(内核)程序的入口地址,将 PC 设置为该地址,CPU 去执行内核中断处理程序
- 中断处理完毕后,CPU 检测到中断返回指令,CPU 从内核态转为用户态,恢复之前的上下文

举例：系统调用

每个操作系统都提供几百种系统调用(进程控制、进程通信、文件使用、目录操作、设备管理、信息维护)

- 当 CPU 执行到特殊的陷入指令的时候(write/read/open/fork/),同样先保存现场,再根据中断向量表,获取中断处理程序,并转交控制权 int 0x80

- 根据查询系统调用表把控制权转给相应的内核函数 fork 2 / read 3 / write 4 / open 5 / close 6 / waitpid 7 / create 8 / link 9 / unlink 10 / execve 11 / chdir 12 / time 13

- 当系统调用中涉及到异步操作的时候,比如等待网卡、磁盘相应这种,CPU 会转而执行其他的进程,当前进程变为阻塞态...由硬件设备发出硬中断信号 ,否则进行下一步

- 中断函数处理完毕后,CPU 检测到中断返回指令,CPU 从内核态转为用户态,恢复之前的上下文

```

// write库函数的实现细节
// liunx/include/unistd.h write原型

#define _syscall3(){
    // 这里调用int 0x80进行系统调用,传递参数系统调用号
}

// int 0x80的实现细节
// IDT(中断向量表)表里取出中断处理函数
// 执行完毕之后回到系统调用的地方,将结果存入寄存器
```

## 进程

进程的创建方式

- 系统初始化(init)
- 正在运行的程序执行了创建进程的系统调用(fork)
- 用户请求创建了一个新的进程(点击可执行文件)
- 初始化一个批处理文件(脚本文件执行

进程的状态

- 运行态 ,指的是进程实际占用 CPU 时间片运行时
- 就绪态 , 就绪态指的是可运行,但因为其他进程正在运行而处于就绪状态
- 阻塞态 , 除非某种外部事件发生,否则进程不能运行

![](./image/runtime.png)

> 进程进行系统调用并不一定都是阻塞的(无需等待外部事件发生),阻塞的原则是需要等待硬件设备的操作.....

多进程的 CPU 图像

1. 进程时间轮片用完
2. 当前进程阻塞

```
// 调度
schedule(){
    // 找出就绪进程队列中的下一个
    pNew = getNext(ReadyQueue)
    // 进行切换
    switch_to(pCur,pNew);
}

// 切换
switch_to(pCur,pNew){
    pCur.ax = CPU.ax;
    pCur.bx = CPU.bx;
    .....
    pCur.cs = CPU.cs;
    pCur.retpc = CPU.pc;

    CPU.ax = pNew.ax;
    CPU.bx = pNew.bx;
    CPU.cs = pNew.cs;
    CPU.retpc = pNew.pc;
}
```

## 内核级线程

CPU 常常说的 8 核 16 个线程，指的就是 CPU 并行的能力,同时可以执行 16 个任务(真正的同时执行)

只有 window 是真正的多线程的,Liunx 系统并未实现线程模型.

在 Liunx 系统中创建进程 fork、创建线程 thread 都是同一个数据结构,只不过不同进程使用的是不同的进程空间,同一个进程中的不同线程使用的是相同的进程空间.

但是底层都是通过 clone()

有阻塞的系统调用

```
// 用户态
main(){
    A();
}
A(){
    B();
}
B(){
    read(); // int 0x80进入内核
}

// 内核态
system_call:
        call sys_read();

sys_read(){

}
```

总结: 这里,其实要明白的就是阻塞的系统调用,要从用户栈 ---> 内核栈 ---> 中断处理函数(阻塞 IO) ----> schedule(CPU 切换不同的内核栈完成进程之间的切换...A,B,C,D)

这时候 DMA 负责去控制硬盘,从硬盘(网卡)中将数据读出来,因为数据不会保留,所以要快速的处理这些数据,硬件发出硬中断告诉 CPU 要快速的处理,CPU 响应硬中断,内核栈中的中断处理函数拿到返回的结果数据,内核栈结束工作后 iret 回用户栈,继续执行用户栈下的指令...

无阻塞的系统调用

```
// 用户态
main(){
    A();
    B();
}
A(){
    fork(); // fork是系统调用,会引起中断
    // 其实你可以理解为所有的进程都是操作系统Fork出来的,而线程跟进程并没有本质的区别
    // move %eax,__NR_fork
    // INT 0x80 // 中断
    // move res,%eax // 结果保存
}

// 内核态
system_call:
    // 把用户态的寄存器的值压入栈 ,保留现场
    push %ds..%fs;
    push %edx....;
    call sys_fork(); // 中断处理函数
    iret; // 这里处理完毕后,返回用户态

_sys_fork:
    call copy process
    ret; // 这里会返回system_call

// copy_process的创建细节

p = (struct task_struct *)get_free_page(); // 获得一页的内存

p->tss.esp0 = PAGE_SIZE + (long) p;
p->tss.ss0 = 0x10;
// 创建内核栈

p->tss.ss = ss & 0xffff;
p->tss.esp = esp;
// 创建用户栈(和父进程共用栈)
```

大致上的流程是这样的:copy process 会首先创建一个新的内核栈,将父进程的内核栈复制一份,赋值给子进程.用户栈都是同一个栈

由于 fork 是非阻塞的,所以,当代码执行完毕的时候,时间片调度到子进程的时候,子进程的代码开始执行,父子进程执行顺序是不确定的....

## CPU 调度

从用户代码开始

```
main(){
    if(!fork()){while(1)printf("A")};
    if(!fork()){while(1)printf("B")};
    wait();
}
```

汇编代码

```
main(){
    move __NR_fork ,%eax
    int 0x80 // fork就是中断系统调用,陷入内核,内核处理完毕后返回
100:move %eax,res // 中断处理完毕后结果保存在res中赋值eax
    cmpl res,0 // 结果跟0比较,如果是父进程,结果不为0,否则为子进程
200:jne 208 // 执行父进程wait()
    printf("A") // 执行子进程的代码,打印A
    jmp 200 // 不断的打印A
208:...
304:wait(); //wait一旦开始执行,主进程让出,使得子进程有机会执行
}
```

一个简单的 wait 示例

```
main(){
    ....
    wait(); // 又是mov __NR_wait int0x80

    // 系统调用函数
    system_call:
        call sys_waitpid

    sys_waitpid(){
        current->state = TASK_INTERRUPTIBL
        schedule() // 开始调度....
    }
}
```

一个实际的调度函数

```
void Schedule(void){
    while(1){
        c = -1;
        next = 0;
        i = NR_TASKS;
        p = &task[NR_TASKS];
        // 遍历PCB数组,找到counter值最大的进程,赋值给c
        while(--i){
            // 如果进程是就绪状态,并且counter值大于-1
            if((*p)->state == TASK_RUNNING && (*p)->counter > c)
                c=(*p)->counter,next=i;
        }
        if(c) break; //找到后就直接跳到switch_to执行
        // 如果就绪的时间片都用完了(counter为0),或者所有的进程都处于阻塞状态
        for(p = &LAST_TASK;p > &FIRST_TASK;--p)
            // 提高阻塞IO的进程的优先级,在下一次调度的时候优先被调度
            (*p)->counter=((*p)->counter>>1) + (*p)->priority;
    }
    switch_to(next);
}
```

Schedule 调用的时机

- 当前运行的进程阻塞了
- 时钟中断

第一次总结:其实忽略那些可有可无的细节问题,我们需要知道的是 CPU 需要在不同的进程之间来回的切换执行,每个进程都有自己的时间轮片,当进程中有需要进行`系统调用`的时候(fork/read/write),CPU 从用户态切换到内核态,CPU 保存用户态的现场后,根据中断向量表,获取该中断相关处理程序,将 PC 设置为该地址后执行.

在内核中执行的时候,像 fork 这种很快会结束,当涉及到 IO 操作的时候,例如文件 read/write,会阻塞当前进程(此时,该进程停留在内核态),转而进行其他进程调度.阻塞的进程会提高优先级,在下一次时间轮片中优先被执行.

当 IO 设备就绪的时候,发送硬中断指令给 CPU,CPU 会立即停下,对应的进程中内核栈中的中断处理函数拿到结果数据,iret 回用户栈,从而继续执行接下来的代码...

Linux 中,对于一个进程,相当于它含有一个(内核)线程.对于多线程来说,原来的进程就是主线程,他们构成了一个线程组.

假设现在进程 Afork()一个子进程 A2,则内核会复制 A 的代码段、数据段、堆、栈四个部分,但这时候虽然他们的虚拟地址是不同的,但是物理内存都是相同的.当父子进程有数据改变的时候,子进程再分配对应的物理空间。从这里我们可以看出来,当不做修改的时候,父子进程是完全一样的..

![](./image/函数栈帧.jpg)

假设我们要在进程 A 中创建一个线程 A2,线程使用进程 A 的虚拟地址和物理地址,代码区是共享的(意味着你在进程中创建的任何函数都可以在所有的线程中执行)

并且所有的线程都可以访问到进程的全局区(全局变量和静态变量),堆区(可以任意访问到 new 的内存地址空间)

![](./image/线程栈.jpg)

栈在线程中都是独立的, 线程之间共享进程的栈,也可以创建自己的栈和堆

![](./image/栈修改.jpg)

**_线程和线程之间是独立的堆栈,但没有限制他的读和写,无论是主线程还是其他线程之间都是可以相互修改的_**

最后,如果程序在运行期间打开一些文件的话,那么进程地址空间中还保存打开的文件的信息,进程的文件也是被所有线程共享的.

![](./image/文件共享.jpg)



## 内存使用与分段(重点学习的地方)

### 1. **编译**

回顾一下,一般一个可执行程序需要经历

- 预处理,删除注释,删除所有的#define 并展开宏定义,插入所有的#include 文件的内容到源文件的对应位置

- 编译,生成汇编语言 （我们直接使用objdump反汇编来看我们的可重定位目标文件即可）

- 可重定位目标文件,汇编转化为机器指令,生成可重定位目标文件,我们把所有的代码放入.text 段,把初始化的全局变量和静态变量放入.data 段,未初始化的数据放入.bss 段,把所有只读的数据放入.rodata 段(字符串常量/const 常量),数据从0x00位置开始存储,按顺序依次存放,代码中对于库函数或者是自定义函数的调用，通常是不知道地址的，直接填0(其实对于大部分的代码基本也可以执行了)



```
gcc -g -c hello.c 生成hello.o 可重定位目标文件
objdump -S hello.o 文件来查看代码段的汇编语言来理解指令的含义


1、 int main () {}

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
; int main () {
       0: 55                            pushq   %rbp   存储父函数栈底
       1: 48 89 e5                      movq    %rsp, %rbp  新函数的栈底是栈顶，待加入变量
       4: 31 c0                         xorl    %eax, %eax  有返回值就处理
; }
       6: 5d                            popq    %rbp   恢复父函数栈底（栈顶指向返回地址）
       7: c3                            retq         栈顶(返回地址) --->  cs:ip

rbp里面存着上一个(main的父函数)的栈底地址,所以,调用main函数的时候,首先父函数栈底地址压入栈

movq %rsp, %rbp 将新创建一个栈帧,将当前的栈顶指针作为当前main函数的栈底,放入%rbp里面

xorl    %eax, %eax  main函数如果有返回值的话,会将返回值放入到%rax中去  

popq    %rbp 将栈顶的值扔到rbp寄存器中,rbp目前指向父函数的栈底

retq 相当于 pop IP 退出main函数, 此时栈中存的是父函数的返回地址，将PC指向返回地址并执行




2. 声明了变量 int main(){int a = 10}


0000000000000000 <_main>:
; int main(){
       0: 55                            pushq   %rbp
       1: 48 89 e5                      movq    %rsp, %rbp
       4: 31 c0                         xorl    %eax, %eax
;     int a = 10;
       6: c7 45 fc 0a 00 00 00          movl    $10, -4(%rbp)
; }
       d: 5d                            popq    %rbp
       e: c3                            retq

这里需要注意的是局部的变量并没有通过push进栈,栈顶依然指向原来rbp的值


3. 有返回值的情况 int main(){int a = 10;return 1;}

0000000000000000 <_main>:
; int main(){
       0: 55                            pushq   %rbp
       1: 48 89 e5                      movq    %rsp, %rbp
       4: c7 45 fc 00 00 00 00          movl    $0, -4(%rbp)
;     int a = 10;
       b: c7 45 f8 0a 00 00 00          movl    $10, -8(%rbp)
;     return 1;
      12: b8 01 00 00 00                movl    $1, %eax  
      17: 5d                            popq    %rbp
      18: c3                            retq

这里其实难以理解的是为啥多了一个0的立即数放入栈空间中去了



4. 声明静态变量和全局变量 int a = 10 ; int main(){ static int b = 20;}

0000000000000008 <_a>:
       8: 0a 00                         orb     (%rax), %al
       a: 00 00                         addb    %al, (%rax)

000000000000000c <_main.b>:
       c: 14 00                         adcb    $0, %al
       e: 00 00                         addb    %al, (%rax)


data段中直接记录,不在栈中存放


5. 执行运算 int main(){int a = 10;int b = 20;int c = a + b;}

0000000000000000 <_main>:
; int main () {
       0: 55                            pushq   %rbp
       1: 48 89 e5                      movq    %rsp, %rbp
       4: 31 c0                         xorl    %eax, %eax
;     int a = 10;
       6: c7 45 fc 0a 00 00 00          movl    $10, -4(%rbp)
;     int b = 20;
       d: c7 45 f8 14 00 00 00          movl    $20, -8(%rbp)
;     int c = a * b;
      14: 8b 4d fc                      movl    -4(%rbp), %ecx // 将a放入ecx寄存器
      17: 0f af 4d f8                   imull   -8(%rbp), %ecx // 将a跟b进行乘法运算，结果保存在ecx
      1b: 89 4d f4                      movl    %ecx, -12(%rbp) // 将结果的值赋值给新的栈空间上即可
; }
      1e: 5d                            popq    %rbp
      1f: c3                            retq

6. 函数参数、调用、返回值

void fun(int x){int y; y = x + 20;return y;}
int main(){int a = 10; int b = fun(a);return 0}


这是一个比较经典的函数调用的例子,我们大概把它看懂了基本也就懂了大概的了


Disassembly of section __TEXT,__text:

0000000000000000 <_fun>:
; int fun(x) {
       0: 55                            pushq   %rbp  // 将main函数的栈帧入栈
       1: 48 89 e5                      movq    %rsp, %rbp // 创建新的栈顶
       4: 89 7d fc                      movl    %edi, -4(%rbp) // 将参数的值入栈
;     y = x + 10;
       7: 8b 45 fc                      movl    -4(%rbp), %eax // 将值赋值给寄存器eax
       a: 83 c0 0a                      addl    $10, %eax  // 和值相加返回eax
       d: 89 45 f8                      movl    %eax, -8(%rbp) // 将得到的结果值入栈
;     return y;
      10: 8b 45 f8                      movl    -8(%rbp), %eax // 将结果值扔到rax寄存器中
      13: 5d                            popq    %rbp // 将rbp的值出栈
      14: c3                            retq      // 
      15: 66 2e 0f 1f 84 00 00 00 00 00 nopw    %cs:(%rax,%rax)
      1f: 90                            nop

0000000000000020 <_main>:
; int main () {
      20: 55                            pushq   %rbp   // 保留父栈帧地址
      21: 48 89 e5                      movq    %rsp, %rbp // 创建main的栈帧,rsp成为新的栈帧地址
      24: 48 83 ec 10                   subq    $16, %rsp   // 分配16字节的栈地址
      28: c7 45 fc 00 00 00 00          movl    $0, -4(%rbp) // 依然是放了0进栈，不知道为啥
;    int a = 10;
      2f: c7 45 f8 0a 00 00 00          movl    $10, -8(%rbp) // int a = 10进栈
;    int b = fun(a);
      36: 8b 7d f8                      movl    -8(%rbp), %edi  将a参数放入参数寄存器
      39: e8 00 00 00 00                callq   0x3e <_main+0x1e> call = push IP  and jump 
      3e: 31 c9                         xorl    %ecx, %ecx  
      40: 89 45 f4                      movl    %eax, -12(%rbp) 将返回值rax的值入栈
;    return 0;
      43: 89 c8                         movl    %ecx, %eax   
      45: 48 83 c4 10                   addq    $16, %rsp  // 重新将栈顶指向起始位置
      49: 5d                            popq    %rbp // 将父栈帧的值弹出并返回
      4a: c3                            retq

```

> 编译阶段,全局的变量全部用00 00 00 00 替代,所有的函数跳转, e8 00 00 00 00 这个位置就是我们即将调用的函数的地址, 当通过链接生成后,这里会替换成我们的最终的函数地址,这个函数地址就是我们链接库找到的函数的地址



- 链接

链接阶段的作用其实还是为全局的变量和函数（本地的函数和外部的函数）找到相应的地址

```c
int global_init_var = 84; // .data段确定虚拟地址后就有了地址
int global_uninit_var; // .bss段确定虚拟地址后就有了地址

void func1(int i){ // .text段
    printf('%d\n',I);   
}
int main(void){ // .text段
    static int static_var = 85;  // .data段
    static int static_var2; // .bss段
    int a = 1;
    int b;
    func1(static_var + static_var + a + b);
    return 0;
}

在符号表中的有:

main 函数(类型为1,text代码段)
func 函数(类型为1,text代码段) 
printf函数(类型为und)
static静态变量(类型为Object)
global_init_var(类型为Object)
global_uninit_var(类型为Object)
```


1. 静态链接（针对那些自定义的函数，多个.c文件分别生成.o文件之后,合并成最终可执行文件，.a实质上就是一堆.o文件以及静态链接库）

```
// a.c
extern int shared;
int main(){
    int a = 100;
    swap(&a,&shared)
}

// b.c
int shared = 1;
void swap(int *a, int *b){
    *a ^= *b = *a ^= *b
}
```

（1）首先链接器会将两个文件的代码段、数据段分别进行合并,此时,代码段合并了main函数和swap函数,数据段中放入了shared变量,默认可执行文件的地址从0x0804800地址开始

（2）计算各个符号的虚拟地址,因为各个符号在段中的相对位置是固定的,所以其实这时候main、shared、swap的地址已经可以确认了

绝对地址修正: shared变量, 源0x000000 -> 0x3000 (实际shared变量在data段的虚拟地址)

相对地址修正: swap函数, 假设swap函数被分配的虚拟地址是0x2000,

需要被修正的位置的地址是0x1000 (main函数的虚拟地址) + 0x27(下条指令的所在位置) = 0x1002b ,调用swap函数的地方

```
调用地址(偏移量) = swap函数的地址（减去位置上的值？） - 下一条指令的地址（0x102b)
```









2. 动态链接(针对库函数调用、系统调用这类型)



> 这时候我得到的就是拥有虚拟地址的可执行文件了,剩下的就是在计算机中执行了


### 2. **在内存中找到一段空闲的区域将程序放入内存并开始取值执行**

首先我们要做的就是将虚拟的地址转化成真实的物理地址

程序在编译的时候是分段的,为了能够高效的在内存中找到空闲的分区,引入了分页的机制.

我们用分页模型来管理物理内存,首先我们把物理内存空间分为 4KB 大小页

针对程序中的段请求,物理内存一页一页(4K)的分配给这个段,避免了浪费

页表 = TLB(缓存寄存器) + 多级页表(未命中的时候直接查询多级页表)

编译程序(分段) -----> 虚拟内存(分区) ----> 物理内存(分页)

虚拟内存的作用就是把程序的段真正映射到物理内存中去

```
ELF代码段 ----> 虚拟内存的代码区
ELF数据段 ----> 虚拟内存的全局区
临时变量/程序员申请 ----> 虚拟内存的堆区和栈区
```

在一个程序看来,所有的指令的逻辑地址和变量的逻辑地址,都需要通过段表找到虚拟内存的地址,然后再通过页表,找到物理地址

当 CPU 调度到这个进程的时候并开始执行,执行的第一条指令发下它是一个虚拟的地址,于是我们去查页表,发现页表里面是硬盘上的地址,于是触发缺页(系统中断),去硬盘中读取到内存,修改页表,等再次去读的时候,通过页表的翻译,定位到物理内存的地址,取出对应的指令.

## 文件与 IO

无论是什么设备,本质上都是 open/read/write/close,只是不同的设备对应着不同的设备文件(/dev/xxx)

```
int fd = open("/dev/xxx")
for(int i=0;i<10;i++){
    write(fd,i,sizeof(int))
}
close(fd)
```

我们从 printf 开始.....
