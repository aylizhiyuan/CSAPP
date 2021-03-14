<!--
 * @Author: lizhiyuan
 * @Date: 2020-11-21 20:53:10
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-03-14 17:42:13
-->
# 操作系统

## 冯诺依曼存储思想

将程序和数据存放到计算机的内部存储器中,计算机在程序控制下一步一步的进行处理

计算机由五大部件组成:输入设备、输出设备、存储器、运算器、控制器

通俗的解释就是:计算机的工作原理是将程序放到内存中,然后用一个指针指向它(cs:ip),然后取指执行,取指执行.....

形象来看的话:当计算机打开一个APP应用的时候,实际是开启了一个进程,该进程会被操作系统自动执行...

总结: 取指 + 执行

## 操作系统启动简略

硬件上电后,操作系统在硬盘上(也有可能是在光盘中)

我们接下来要做的事儿就是把磁盘上的代码放入内存中,将cs:ip的指针指向它,让计算机取指执行

BIOS会先读取磁盘上的0磁道0扇区(1个扇区) ----> 0x07c00 引导扇区 bootsect.s (这时候会将07c00的内存空间腾出来,放到0x90000处执行....)

0x13中断继续读磁盘中的第2-5个扇区(4个扇区) ----> 0x90200(bootsect 512个字节,0x90000 + 512个字节  = 0x90200) setup.s

setup.s会将操作系统的代码移动到0地址--0x07c00的内存地址上去

最后setup.s会切换到保护模式,32位(寻址)模式.跳到0地址去执行操作系统的代码

***总结:BIOS ----> bootsect.s ----> setup.s -----> system***

main函数开始初始化操作系统,main函数永不退出,永不返回....
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


## 中断/异常

中断是异步操作....CPU和硬件设备是可以并行操作的,CPU不必等待硬件IO设备的操作结果,当设备完成的时候,由设备发出中断信号告知CPU,CPU立即保存当前的执行情况后处理。。。

这就是操作系统首次实现异步的一个机制: 中断机制

- IO中断
- 时钟中断 进程轮片/定时
- 硬件故障

异常其实也是种中断,只不过通常异常是CPU主动触发的一种中断

- 系统调用 
- 页故障/页错误
- 保护性异常 (读写冲突) 
- 断点调试
- 其他程序异常

总结: 中断是一些外部事件,被动触发的,异常是由正在执行的指令引起的，属于主动触发的....


## 工作原理

CPU会在每条指令执行周期的最后时刻扫描中断寄存器,查看是否有中断信号,若有中断,通过查找中断向量表引出中断处理函数

- 128 0x80 系统调用异常中断
- 32 - 127 外部中断,IO设备中断


举例：IO设备硬中断 

- 打印机给CPU发送中断信号
- CPU处理完当前指令后检测到中断,判断出中断来源并向相关设备发确认信号
- CPU进行`系统调用`,切换到内核态，并将现场保存(程序计数器PC以及程序状态字PSW)
- CPU根据中断向量表,获得该中断相关处理程序(内核)程序的入口地址,将PC设置为该地址,CPU去执行内核中断处理程序
- 中断处理完毕后,CPU检测到中断返回指令,CPU从内核态转为用户态,恢复之前的上下文


举例：系统调用 

每个操作系统都提供几百种系统调用(进程控制、进程通信、文件使用、目录操作、设备管理、信息维护)

- 当CPU执行到特殊的陷入指令的时候(write/read/open/fork/),同样先保存现场,再根据中断向量表,获取中断处理程序,并转交控制权 int 0x80 

- 根据查询系统调用表把控制权转给相应的内核函数 fork 2 / read 3 / write 4 / open 5 / close 6 / waitpid 7 / create 8 / link 9 / unlink 10 / execve 11 / chdir 12 / time 13 

- 当系统调用中涉及到异步操作的时候,比如等待网卡、磁盘相应这种,CPU会转而执行其他的进程,当前进程变为阻塞态...由硬件设备发出硬中断信号 ,否则进行下一步

- 中断函数处理完毕后,CPU检测到中断返回指令,CPU从内核态转为用户态,恢复之前的上下文

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

- 运行态 ,指的是进程实际占用CPU时间片运行时
- 就绪态 , 就绪态指的是可运行,但因为其他进程正在运行而处于就绪状态
- 阻塞态 , 除非某种外部事件发生,否则进程不能运行

![](./image/runtime.png)

> 进程进行系统调用并不一定都是阻塞的(无需等待外部事件发生),阻塞的原则是需要等待硬件设备的操作.....

多进程的CPU图像

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

## 用户级线程

这部分内容就不再主动去了解和深入了...通过c库中的pthread调用产生的都是内核级别的线程....

## 内核级线程

CPU常常说的8核16个线程，指的就是CPU并行的能力,同时可以执行16个任务(真正的同时执行)

只有window是真正的多线程的,Liunx系统并未实现线程模型.

在Liunx系统中创建进程fork、创建线程thread都是同一个数据结构,只不过不同进程使用的是不同的进程空间,同一个进程中的不同线程使用的是相同的进程空间.

但是底层都是通过clone()

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

总结: 这里,其实要明白的就是阻塞的系统调用,要从用户栈 ---> 内核栈 ---> 中断处理函数(阻塞IO) ----> schedule(CPU切换不同的内核栈完成进程之间的切换...A,B,C,D)

这时候DMA负责去控制硬盘,从硬盘(网卡)中将数据读出来,因为数据不会保留,所以要快速的处理这些数据,硬件发出硬中断告诉CPU要快速的处理,CPU响应硬中断,内核栈中的中断处理函数拿到返回的结果数据,内核栈结束工作后iret回用户栈,继续执行用户栈下的指令...


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

大致上的流程是这样的:copy process会首先创建一个新的内核栈,将父进程的内核栈复制一份,赋值给子进程.用户栈都是同一个栈

由于fork是非阻塞的,所以,当代码执行完毕的时候,时间片调度到子进程的时候,子进程的代码开始执行,父子进程执行顺序是不确定的....


## CPU调度

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

一个简单的wait示例

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

Schedule调用的时机

- 当前运行的进程阻塞了
- 时钟中断


第一次总结:其实忽略那些可有可无的细节问题,我们需要知道的是CPU需要在不同的进程之间来回的切换执行,每个进程都有自己的时间轮片,当进程中有需要进行`系统调用`的时候(fork/read/write),CPU从用户态切换到内核态,CPU保存用户态的现场后,根据中断向量表,获取该中断相关处理程序,将PC设置为该地址后执行.

在内核中执行的时候,像fork这种很快会结束,当涉及到IO操作的时候,例如文件read/write,会阻塞当前进程(此时,该进程停留在内核态),转而进行其他进程调度.阻塞的进程会提高优先级,在下一次时间轮片中优先被执行.

当IO设备就绪的时候,发送硬中断指令给CPU,CPU会立即停下,对应的进程中内核栈中的中断处理函数拿到结果数据,iret回用户栈,从而继续执行接下来的代码...

Linux中,对于一个进程,相当于它含有一个(内核)线程.对于多线程来说,原来的进程就是主线程,他们构成了一个线程组.

进程和线程对操作系统来说,都是一样的,同样拷贝一份内核栈,区别是用户栈是共享还是复制



## 进程同步与信号量

这部分的内容对于目前的我来说,不是很重要,暂时忽略把...

## 死锁处理

同样不重要,闲暇时间补充把....

## 内存使用与分段(重点学习的地方)

###  1. **编译** 

回顾一下,一般一个可执行程序需要经历

- 预处理,删除注释,删除所有的#define并展开宏定义,插入所有的#include 文件的内容到源文件的对应位置

- 编译,生成汇编语言

- 汇编转化为机器指令,生成目标文件,我们把所有的代码放入.text段,把初始化的全局变量和静态变量放入.data段,未初始化的数据放入.bss段,把所有只读的数据放入.rodata段(字符串常量/const常量)

- 链接（这个阶段暂时可以不用细究）

把一个可执行程序,首先在内存中找到一个空闲内存,将空闲内存的基地址放入PCB中,然后把程序放到里面去,每次取值执行的时候,进行地址翻译,基地址 + 偏移地址 = 实际的物理内存地址

这样,程序在编译的时候可以不用考虑使用实际的物理内存,当程序在运行的时候 ---- 创建进程运行,再去申请内存

这点是很关键的,当CPU去执行的时候,找到代码段取值执行,如果要创建变量就直接去堆区和栈区写入即可...


### 2. **在内存中找到一段空闲的区域将程序放入内存并开始取值执行**

程序在编译的时候是分段的,为了能够高效的在内存中找到空闲的分区,引入了分页的机制.----实际的物理内存是分页


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

当CPU调度到这个进程的时候并开始执行,执行的第一条指令发下它是一个虚拟的地址,于是我们去查页表,发现页表里面是硬盘上的地址,于是触发缺页(系统中断),去硬盘中读取到内存,修改页表,等再次去读的时候,通过页表的翻译,定位到物理内存的地址,取出对应的指令.

## 文件与IO






PS: 能够将操作系统的大致逻辑弄清楚,有助于自己对运行原理的理解,剩下的,希望自己可以在看源码/创新项目中不断的打磨把...共勉!!




















