// Ping-pong a counter between two processes.
// Only need to start one of these -- splits into two with fork.

/* 这段代码是一个简单的 ping-pong 测试，两个进程（父进程和子进程）之间通过 IPC
(Inter-Process Communication，进程间通信) 交换信息。

首先，代码定义了四个静态整数数组 A、B、C 和 D，它们都被初始化为 0。

在主函数 main() 中，首先定义了一些变量，包括 who（用于存储 fork
的返回值，即子进程的 PID）、i（用于接收 IPC
消息）、isSon（用于标识当前进程是否为子进程）、ptr、father 和 perm。

接着，使用 memset 函数将数组 A、B、C 和 D 的所有元素都设置为 0。

然后，调用 syscall_getenvid() 函数获取当前进程的环境 ID，并将其存储在 father
变量中。

接下来，调用 fork() 函数创建一个子进程。如果当前进程是父进程（即 fork()
的返回值不为 0），则通过 ipc_send() 函数向子进程发送一个值为 0 的 IPC
消息。如果当前进程是子进程（即 fork() 的返回值为 0），则将 isSon 设置为 1。

最后，进入一个无限循环，在循环中，每个进程都会调用 ipc_recv() 函数等待接收 IPC
消息。当接收到消息后，将消息的值存储在 i 变量中，并打印出当前进程的环境 ID
和接收到的消息。

这个程序的主要目的是测试和演示进程间的通信，以及 fork() 函数的使用。
 */
#include <lib.h>

static int A[1024] __attribute__((aligned(4096)));
static int B[1024] __attribute__((aligned(4096)));
static int C[1024] __attribute__((aligned(4096)));
static int D[1024] __attribute__((aligned(4096)));

int main() {
	u_int who, i;
	int isSon = 0;
	int *ptr;
	int father;
	u_int perm;

	memset(A, 0, PAGE_SIZE);
	memset(B, 0, PAGE_SIZE);
	memset(C, 0, PAGE_SIZE);
	memset(D, 0, PAGE_SIZE);
	father = syscall_getenvid();

	if ((who = fork()) != 0) {
		// get the ball rolling
		debugf("@@@@@send 0 from %x to %x\n", syscall_getenvid(), who);
		ipc_send(who, 0, 0, 0);
		// user_panic("&&&&&&&&&&&&&&&&&&&&&&&&m");
	} else {
		isSon = 1;
	}

	for (;;) {
		debugf("@@@@%x am waiting.....\n", syscall_getenvid());
		i = ipc_recv(&who, 0, 0);

		debugf("@@@@%x got %d from %x\n", syscall_getenvid(), i, who);

		// user_panic("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&");
		if (i == 10) {
			break;
		}

		i++;
		debugf("@@@@@send %d from %x to %x\n", i, syscall_getenvid(), who);
		ipc_send(who, i, 0, 0);

		if (i == 10) {
			break;
		}
	}

	debugf("@@@@ %x finish test 1\n", syscall_getenvid());

	//--------------------------------------------------------------------------------

	// int *ptr = ROUNDWON(B, PAGE_SIZE);
	// ptr = ROUNDDOWN(B, PAGE_SIZE);

	if (isSon) {
		if ((who = fork()) == 0) {
			// grandSon
			debugf("grandson\n");
			debugf("@@@@grandson %x waiting for son's envid\n",
				   syscall_getenvid());
			i = ipc_recv(&who, 0, 0);

			debugf("@@@@grandson %x got envid %x from son %x\n", syscall_getenvid(), i,
			       who);

			ptr = (int *)ROUNDDOWN(C, PAGE_SIZE);
			ptr[0] = 2020;
			ptr[100] = syscall_getenvid();
			ptr[200] = i;
			ptr[300] = 4;
			debugf("@@@@grandson %x send Page to son %x\n", syscall_getenvid(), i);
			ipc_send(i, 0, ptr, PTE_D | PTE_LIBRARY);

			debugf("@@@@grandson %x is waiting ack from son %x\n", syscall_getenvid(),
			       i);
			i = ipc_recv(&who, 0, 0);
			if (i != 0) {
				user_panic("Error when grandson get ack from son!");
			}
			debugf("@@@@grandson %x got ack from son %x\n", syscall_getenvid(),
			who);

			debugf("@@@@2 %d lab %d strong testcase2\n", ptr[300], ptr[0]);
			debugf("@@@@2 send is %x recv is %x\n", ptr[100], ptr[200]);
			debugf("@@@@grandson %x send continue to son %x\n", syscall_getenvid(),
			       who);
			ipc_send(who, 0, 0, 0);

			debugf("@@@@grandson %x wait for father signal\n", syscall_getenvid());
			i = ipc_recv(&who, 0, 0);
			if (i != 0) {
				user_panic("Error when grandson get ack from father!");
			}
			debugf("@@@@4 %d lab %d strong testcase4\n", ptr[100], ptr[200]);
			debugf("@@@@4 send is %x recv is %x\n", ptr[0], ptr[300]);

		} else {
			// Son
			debugf("@@@@son tell grandson his envid %x\n", syscall_getenvid());
			ipc_send(who, syscall_getenvid(), 0, 0);

			debugf("@@@@son %x is waiting from grandSon %x page\n", syscall_getenvid(),
			       who);

			ptr = (int *)ROUNDDOWN(B, PAGE_SIZE);
			i = ipc_recv(&who, ptr, &perm);
			if (i != 0) {
				user_panic("Error when son get Page from grandson!");
			}
			debugf("ptr is %08x\n", ptr);
			debugf("@@@@1 %d lab %d strong testcase1\n", ptr[0], ptr[300]);
			debugf("@@@@1 send is %x recv is %x\n", ptr[100], ptr[200]);

			ptr[0] = 4;
			ptr[100] = syscall_getenvid();
			ptr[200] = who;
			ptr[300] = 2020;
			debugf("@@@@son %x send ack to grandson %x\n", syscall_getenvid(), who);
			ipc_send(who, 0, 0, 0);
			debugf("@@@@son %x wait continue from grandson %x\n", syscall_getenvid(),
			       who);
			i = ipc_recv(&who, 0, 0);
			if (i != 0) {
				user_panic("Error when son get continue from grandson");
			}

			debugf("@@@@son %x send Page to father %x\n", syscall_getenvid(), father);
			ipc_send(father, 0, ptr, perm);
			debugf("@@@@son %x tell father grandson id %x\n", syscall_getenvid(), who);
			ipc_send(father, who, 0, 0);
			debugf("@@@@son %x wait for father %x signal\n", syscall_getenvid(),
			       father);
			i = ipc_recv(&who, 0, 0);
			if (i != 0) {
				user_panic("Error when wait for father signal!");
			}
			debugf("@@@@3 %d lab %d strong testcase3\n", ptr[100], ptr[200]);
			debugf("@@@@3 send is %x recv is %x\n", ptr[0], ptr[300]);
		}
	} else {
		// father
		ptr = (int *)ROUNDDOWN(D, PAGE_SIZE);
		debugf("@@@@father %x wait for son page\n", syscall_getenvid());
		i = ipc_recv(&who, ptr, &perm);
		if (i != 0) {
			user_panic("Error when father get Page from son");
		}
		debugf("@@@@father %x wait for grandson id\n", syscall_getenvid());
		i = ipc_recv(&who, 0, 0);
		// if (i != 0) {
		//  user_panic("Error when father wait for grandson id");
		// }
		debugf("@@@@father %x got grandson id %x from son %x\n", syscall_getenvid(), i,
		       who);
		ptr[100] = 2020;
		ptr[200] = 4;
		ptr[0] = syscall_getenvid();
		ptr[300] = who;
		ptr[300] = i;
		// The answer was provided as if i is written before son finishes recv, so we do it
		// earlier to avoid sched racing. To enhance the case, we may modify ptr[300] after
		// the son sends an ack.
		debugf("@@@@father send signal to son\n");
		ipc_send(who, 0, 0, 0);
		debugf("@@@@father send signal to grandson\n");
		ipc_send(i, 0, 0, 0);
	}
	debugf("@@@@I am killing %x\n", syscall_getenvid());

	return 0;
}
