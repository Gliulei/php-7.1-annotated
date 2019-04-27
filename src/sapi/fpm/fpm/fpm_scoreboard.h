
	/* $Id: fpm_status.h 312263 2011-06-18 17:46:16Z felipe $ */
	/* (c) 2009 Jerome Loyet */

#ifndef FPM_SCOREBOARD_H
#define FPM_SCOREBOARD_H 1

#include <sys/time.h>
#ifdef HAVE_TIMES
#include <sys/times.h>
#endif

#include "fpm_request.h"
#include "fpm_worker_pool.h"
#include "fpm_atomic.h"

#define FPM_SCOREBOARD_ACTION_SET 0 //重置操作
#define FPM_SCOREBOARD_ACTION_INC 1 //增加操作

struct fpm_scoreboard_proc_s { //对应各worker进程的详细信息
	union {
		atomic_t lock; //保证原子性的锁机制
		char dummy[16];
	};
	int used; //是否被使用
	time_t start_epoch;
	pid_t pid; //进程id
	unsigned long requests;
	enum fpm_request_stage_e request_stage; //请求处理阶段
	struct timeval accepted; //accept请求的时间
	struct timeval duration; //脚本执行的时间
	time_t accepted_epoch; //accept请求时间戳
	struct timeval tv; //活跃时间
	char request_uri[128]; //请求URI
	char query_string[512]; //请求参数
	char request_method[16];  //请求方法
	size_t content_length; /* used with POST only */ //请求长度
	char script_filename[256];
	char auth_user[32];
#ifdef HAVE_TIMES
	struct tms cpu_accepted;
	struct timeval cpu_duration;
	struct tms last_request_cpu;
	struct timeval last_request_cpu_duration;
#endif
	size_t memory; //内存使用大小
};

struct fpm_scoreboard_s {
	union { //保证原子性的锁机制
		atomic_t lock;
		char dummy[16];
	};
	char pool[32]; //worker名称
	int pm; //运行模式
	time_t start_epoch; //开始的时间
	int idle; //process的空闲数
	int active; //process的活跃数
	int active_max; //最大活跃数
	unsigned long int requests; //请求次数
	unsigned int max_children_reached; //达到最大进程数限制的次数
	int lq;               //当前listen queue的请求数
	int lq_max;           //listen queue的大小
	unsigned int lq_len;  //listen queue的长度
	unsigned int nprocs;  //process的总数
	int free_proc;  //从process的列表遍历下一个空闲对象的开始下标
	unsigned long int slow_rq; //慢请求数
	struct fpm_scoreboard_proc_s *procs[]; //计分板详情
};

int fpm_scoreboard_init_main();
int fpm_scoreboard_init_child(struct fpm_worker_pool_s *wp);

void fpm_scoreboard_update(int idle, int active, int lq, int lq_len, int requests, int max_children_reached, int slow_rq, int action, struct fpm_scoreboard_s *scoreboard);
struct fpm_scoreboard_s *fpm_scoreboard_get();
struct fpm_scoreboard_proc_s *fpm_scoreboard_proc_get(struct fpm_scoreboard_s *scoreboard, int child_index);

struct fpm_scoreboard_s *fpm_scoreboard_acquire(struct fpm_scoreboard_s *scoreboard, int nohang);
void fpm_scoreboard_release(struct fpm_scoreboard_s *scoreboard);
struct fpm_scoreboard_proc_s *fpm_scoreboard_proc_acquire(struct fpm_scoreboard_s *scoreboard, int child_index, int nohang);
void fpm_scoreboard_proc_release(struct fpm_scoreboard_proc_s *proc);

void fpm_scoreboard_free(struct fpm_scoreboard_s *scoreboard);

void fpm_scoreboard_child_use(struct fpm_scoreboard_s *scoreboard, int child_index, pid_t pid);

void fpm_scoreboard_proc_free(struct fpm_scoreboard_s *scoreboard, int child_index);
int fpm_scoreboard_proc_alloc(struct fpm_scoreboard_s *scoreboard, int *child_index);

#ifdef HAVE_TIMES
float fpm_scoreboard_get_tick();
#endif

#endif
