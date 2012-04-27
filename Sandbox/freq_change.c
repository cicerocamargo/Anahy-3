#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <ctype.h>

#define pprintf(level, ...) do { \
	if (level <= verbosity) { \
		if (daemonize) \
			syslog(LOG_INFO,__VA_ARGS__); \
		else \
			printf(__VA_ARGS__); \
	} \
} while(0)

#define SYSFS_TREE "/sys/devices/system/cpu/"
#define SYSFS_SETSPEED "scaling_setspeed"
#define VERSION "0.75"

typedef struct cpustats {
	unsigned long long user;
	unsigned long long mynice;
	unsigned long long system;
	unsigned long long idle;
	unsigned long long iowait;
	unsigned long long irq;
	unsigned long long softirq;
} cpustats_t;

typedef struct cpuinfo {
	unsigned int cpuid;
	unsigned int nspeeds;
	unsigned int max_speed;
	unsigned int min_speed;
	unsigned int current_speed;
	unsigned int speed_index;
	unsigned int current_level;
	int fd;
	char *sysfs_dir;
	cpustats_t *last_reading;
	cpustats_t *reading;
	int in_mhz; /* 0 = in kHz, 1 = in mHz */
	unsigned long *freq_table;
	int table_size;
	int threads_per_core;
	int scalable_unit;
} cpuinfo_t;

enum function {
	SINE,
	AGGRESSIVE,
	PASSIVE,
	LEAPS
} func = PASSIVE;

enum modes {
	LOWER,
	SAME,
	RAISE
};

// Global matrix that contains the pointer to all the cpu information

cpuinfo_t **all_cpus;

// For a daemon as simple as this, global data is ok.

int daemonize = 1;
int ignore_nice = 1;
int verbosity = 0;
unsigned int step = 100000;  /* in kHz */
unsigned int poll = 1000; /* in msecs */
unsigned int highwater = 0;
unsigned int lowwater = 0;
unsigned int max_limit = 0;
unsigned int min_limit = 0;
unsigned int step_specified = 0;
unsigned int t_per_core = 1;
unsigned int cores_specified = 0;
int selector_cpu = -1;

static char buf[1024];

unsigned int change_speed_count = 0;
time_t start_time = 0;

void help(void) {
	printf("\nOpcoes:\n");
	printf(" 	-h	Exibe esta mensagem de ajuda\n");
	printf("	-d	(Default), permanece rodando no terminal, executa na pratica em 'loop'\n");
	printf("	-v	Exibe as definicoes atuais, numero de CPU's do sistema com suas possiveis frequencias.\n");
	printf("	-n	Mostra a porcentagem de uso das cpu's no momento (deve ser usado junto com -d)\n");
	printf("	-m #	Modos de operacao, pode ser 0, 1, 2, ou 3:\n");
	printf("			0 = MESMO (mantem), 1 = AGRESSIVO (elevar)\n");
	printf("			2 = PASSIVO (reduzir), 3 = SALTA (apenas muda)\n");
	printf("			O padrao atual encontra-se em PASSIVO para economia de energia\n");
	printf("	-s #	Gera uma escala maior de frequencias\n");
	printf("	-u # 	Especifica qual das CPU's existentes queremos alterar a frequencia\n");
	printf("	-c #	Especifica num. de threads por core (ja ha algo que faca isso por voce)\n");
	printf("	-e #	CPU (core) usa limite para elevar: porcentagem de [0..100]\n");
	printf("	-b #    CPU (core) usa limite para baixar: porcentagem de [0..100]\n");
	printf("\n");

	return;
}

// Opens a file and copy its first 1024 bytes for the global 'buf'

int read_file(const char *file, int fd, int new) {
	int n, err;
	
	if (new) {
		if ((fd = open(file, O_RDONLY)) == -1) {
			err = errno;
			perror(file);
			return err;
		}
	}
	
	lseek(fd, 0, SEEK_SET);
	if ((n = read(fd, buf, sizeof(buf)-1)) < 0) {
		err = errno;
		perror(file);
		close(fd);
		return err;
	}
	buf[n] = '\0';

	if (new)
		close(fd);
	
	return 0;
}

/* Read /proc/stat and send it to the buf, then divide the output into pieces
   Format of line it's:
 * 
 * cpu<id> <user> <nice> <system> <idle> <iowait> <irq> <softirq>
 */

int get_stat(cpuinfo_t *cpu) {

	char *p1, *p2, searchfor[10];
	int err;
	
	if ((err = read_file("/proc/stat", cpu->fd, 0)) != 0) {
		return err;
	}

	sprintf(searchfor, "cpu%d ", cpu->cpuid);

	p1 = strstr(buf, searchfor);
	if (p1 == NULL)	{
		perror("Erro ao analisar /proc/stat");
		return ENOENT;
	}
	
	p2 = p1+strlen(searchfor);

	memcpy(cpu->last_reading, cpu->reading, sizeof(cpustats_t));
	
	cpu->reading->user = strtoll(p2, &p2, 10);
	cpu->reading->mynice = strtoll(p2, &p2, 10);
	cpu->reading->system = strtoll(p2, &p2, 10);
	cpu->reading->idle = strtoll(p2, &p2, 10);
	cpu->reading->iowait = strtoll(p2, &p2, 10);
	cpu->reading->irq = strtoll(p2, &p2, 10);
	cpu->reading->softirq = strtoll(p2, &p2, 10);

	return 0;
}

// Once the dicision has been made, the speed is changed

int change_speed(cpuinfo_t *cpu, enum modes mode) {
	int fd, len, err, i;
	cpuinfo_t *save;
	char writestr[100];
	int muda, atual, mtr[cpu->table_size];

	if (cpu->cpuid != cpu->scalable_unit) 
		return 0;
	for(i = 0 ; i<cpu->table_size ; i++) {
		mtr[i] = ((cpu->freq_table[i]*100) / cpu->freq_table[0]);

		if (mtr[i] == highwater)
			muda = i;
		if (mtr[i] == lowwater)
			muda = i;
		if(mtr[i] == cpu->current_level)
			atual = i;
	}
	cpu->speed_index = atual;
	if (mode == RAISE) {
		if ((func == AGGRESSIVE) || (func == LEAPS)) {
			cpu->speed_index = muda;
		} else {
			if (cpu->speed_index != muda) 
				cpu->speed_index--;
		} 
	} else {
		if ((func == PASSIVE) || (func == LEAPS)) {
			cpu->speed_index = muda;
		} else {
			if (cpu->speed_index != muda)
				cpu->speed_index++;
		}
	}
	/* 
	 * We've got adjust the current speed in all virtual CPUs that fall 
         * into CPU's scalable unit
	*/
	
	save = cpu;
	for (i = save->cpuid; i < (save->cpuid + save->threads_per_core); i++) {
		cpu = all_cpus[i];
		cpu->current_speed = save->freq_table[save->speed_index];
	}
	
	cpu = save;

        pprintf(3,"Definir a velocidade para %d\n", cpu->current_speed);
	printf("cpu%d, Definido: %d Mhz\n",cpu->cpuid, cpu->current_speed/1000);
	change_speed_count++;
	
	strncpy(writestr, cpu->sysfs_dir, 50);
	strncat(writestr, SYSFS_SETSPEED, 20);

	if ((fd = open(writestr, O_WRONLY)) < 0) {
		err = errno;
		perror("Nao e possivel abrir scaling_setspeed");
		return err;
	}

	lseek(fd, 0, SEEK_CUR);

	sprintf(writestr, "%d\n", (cpu->in_mhz) ?
			(cpu->current_speed / 1000) : cpu->current_speed); 

	pprintf(4,"modo=%d, str=%s", mode, writestr);
	
	if ((len = write(fd, writestr, strlen(writestr))) < 0) {
		err = errno;
		perror("Impossivel escrever para scaling_setspeed\n");
		return err;
	}

	if (len != strlen(writestr)) {
		printf("Impossivel escrever em scaling_setspeed\n");
		close(fd);
		return EPIPE;
	}
	close(fd);

	return 0;
}

// This is the principal program point

enum modes inline decide_speed(cpuinfo_t *cpu) {
	int err;
	float pct;
	unsigned long long usage, total;
	
	if ((err = get_stat(cpu)) < 0) {
		perror("Nao pode pegar o status");
		return err;
	}

	total = (cpu->reading->user - cpu->last_reading->user) +
		(cpu->reading->system - cpu->last_reading->system) +
		(cpu->reading->mynice - cpu->last_reading->mynice) +
		(cpu->reading->idle - cpu->last_reading->idle) +
		(cpu->reading->iowait - cpu->last_reading->iowait) +
		(cpu->reading->irq - cpu->last_reading->irq) +
		(cpu->reading->softirq - cpu->last_reading->softirq);
	if (!ignore_nice) { 
		usage = (cpu->reading->user - cpu->last_reading->user) +
			(cpu->reading->system - cpu->last_reading->system) +
			(cpu->reading->irq - cpu->last_reading->irq) +
			(cpu->reading->softirq - cpu->last_reading->softirq);
	} else {
		usage = (cpu->reading->user - cpu->last_reading->user) +
			(cpu->reading->mynice - cpu->last_reading->mynice) +
			(cpu->reading->system - cpu->last_reading->system) +
			(cpu->reading->irq - cpu->last_reading->irq) +
			(cpu->reading->softirq - cpu->last_reading->softirq);
	}
	if(!ignore_nice) {
		return pct = ((float)usage)/((float)total) * 100;
		printf("cpu%d = %.1f%%\n",cpu->cpuid, pct*100);
	}
	if ((highwater != 0) && (highwater >= cpu->current_level) && 
			(cpu->current_speed != cpu->max_speed)) {
		// eleva a velocidade para o proximo nivel.
		pprintf(6, "Elevando (RAISE)\n"); 
		return RAISE;
	} else if ((lowwater != 0) && (lowwater <= cpu->current_level) && 
			(cpu->current_speed != cpu->min_speed)) {
		// baixa a velocidade.
		pprintf(6, "Baixando (LOWER)\n");
		return LOWER;
	}
	return SAME;
}

// Compare the functions to raffle one frequency list in ascending order

int faked_compare(const void *a, const void *b) {
	unsigned long *a1 = (unsigned long *)a;
	unsigned long *b1 = (unsigned long *)b;

	if (*a1 < *b1) return 1;
	if (*a1 > *b1) return -1;

	return 0;
}

// Allocate and initialize the CPU's strutures

int get_per_cpu_info(cpuinfo_t *cpu, int cpuid) {
	char cpustr[100], scratch[100], tmp[11], *p1;
	int fd, err;
	unsigned long temp;

	cpu->cpuid = cpuid;
	cpu->sysfs_dir = (char *)malloc(50*sizeof(char));
	if (cpu->sysfs_dir == NULL) {
		perror("Nao foi possivel alocar per-cpu sysfs_dir");
		return ENOMEM;
	}
	memset(cpu->sysfs_dir, 0, (50*sizeof(char)));

	strncpy(cpu->sysfs_dir, SYSFS_TREE, 30);
	sprintf(cpustr, "cpu%d/cpufreq/", cpuid);
	strncat(cpu->sysfs_dir, cpustr, 20);
	
	strncpy(scratch, cpu->sysfs_dir, 50);
	strncat(scratch, "cpuinfo_max_freq", 18);
	if ((err = read_file(scratch, 0, 1)) != 0) {
		return err;
	}

	cpu->max_speed = strtol(buf, NULL, 10);
	
	strncpy(scratch, cpu->sysfs_dir, 50);
	strncat(scratch, "cpuinfo_min_freq", 18);

	if ((err = read_file(scratch, 0, 1)) != 0) {
		return err;
	}

	cpu->min_speed = strtol(buf, NULL, 10);
	
        /* 
	 * More error handling, make sure step is not larger than the 
	 * difference between max and min speeds. If so, truncate it.
	 */
	
        if (step > (cpu->max_speed - cpu->min_speed)) {
		step = cpu->max_speed - cpu->min_speed;
	}
	
        cpu->current_speed = cpu->max_speed;
	cpu->speed_index = 0;

	strncpy(scratch, cpu->sysfs_dir, 50);
	strncat(scratch, "scaling_available_frequencies", 50);

	if (((err = read_file(scratch, 0, 1)) != 0) || (step_specified)) {

                /* 
		 * We don't have scaling_available_frequencies. build the
		 * table from the min, max, and step values.  the driver
		 * could ignore these, but we'll represent it this way since
		 * we don't have any other info.
		*/
	
                cpu->table_size = ((cpu->max_speed-cpu->min_speed)/step) + 1;
		cpu->table_size += ((cpu->max_speed-cpu->min_speed)%step)?1:0;
		
		cpu->freq_table = (unsigned long *)
			malloc(cpu->table_size*sizeof(unsigned long));

		if (cpu->freq_table == (unsigned long *)NULL) {
			perror("Nao e possivel alocar cpu->freq_table");
			return ENOMEM;
		}
		/* populate the table. Start at the top, and subtract step*/
		
                for (temp = 0; temp < cpu->table_size; temp++) {
			cpu->freq_table[temp] = 
			((cpu->min_speed<(cpu->max_speed-(temp*step))) ? 
			 (cpu->max_speed-(temp*step)) :
			 (cpu->min_speed) );
		}	
	} else {
		
        /* 
		 * We do have the file, parse it and build the table from
		 * there.
		 * 
		 * The format of scaling_available_frequencies (SAF) is:
		 * "number<space>number2<space>...numberN<space>\n", but this
		 * can change. So we're relying on the fact that strtoul will 
		 * return 0 if it can't find anything, and that 0 will never 
		 * be a real value for the available frequency. 
		 */
		p1 = buf;
		
		temp = strtoul(p1, &p1, 10);
		while((temp > 0) && (cpu->table_size < 100)) {
			cpu->table_size++;
			temp = strtoul(p1, &p1, 10);
		}
	
		cpu->freq_table = (unsigned long *)
			malloc(cpu->table_size*sizeof(unsigned long));
		if (cpu->freq_table == (unsigned long *)NULL) {
			perror("Nao foi possivel alocar cpu->freq_table\n");
			return ENOMEM;
		}
	
		p1 = buf;
		for (temp = 0; temp < cpu->table_size; temp++) {
			cpu->freq_table[temp] = strtoul(p1, &p1, 10);
		}
	}
	
	/* now lets sort the table just to be sure */
        
	qsort(cpu->freq_table, cpu->table_size, sizeof(unsigned long), &faked_compare);

	strncpy(scratch, cpu->sysfs_dir, 50);
	strncat(scratch, "scaling_governor", 20);

	if ((err = read_file(scratch, 0, 1)) != 0) {
		perror("Nao pode abrir o arquivo scaling_governors");
		return err;
	}

	if (strncmp(buf, "userspace", 9) != 0) {
		if ((fd = open(scratch, O_RDWR)) < 0) {
			err = errno;
			perror("Nao pode abrir do ficheiro para escreita");
			return err;
		}
		strncpy(tmp, "userspace\n", 11);
		if (write(fd, tmp, 11*sizeof(char)) < 0) {
			err = errno;
			perror("Erro ao gravar o arquivo governador");
			close(fd);
			return err;
		}
		if ((err = read_file(scratch, fd, 0)) != 0) {
			perror("Erro ao ler o arquivo governador de volta");
			close(fd);
			return err;
		}
		close(fd);
		if (strncmp(buf, "userspace", 9) != 0) {
			perror("Nao pode definir a userspace governor, saindo");
			return EPIPE;
		}
	}

	cpu->last_reading = (cpustats_t *)malloc(sizeof(cpustats_t));
	cpu->reading = (cpustats_t *)malloc(sizeof(cpustats_t));
	memset(cpu->last_reading, 0, sizeof(cpustats_t));
	memset(cpu->reading, 0, sizeof(cpustats_t));
	
	/*
	 * Some cpufreq drivers (longhaul) report speeds in MHz instead
	 * of KHz.  Assume for now that any currently supported cpufreq 
	 * processor will a) not be faster then 10GHz, and b) not be slower
	 * then 10MHz. Therefore, is the number for max_speed is less than
	 * 10000, assume the driver is reporting speeds in MHz, not KHz,
	 * and adjust accordingly.
	 */
        
	cpu->in_mhz = 0;
	if (cpu->max_speed <= 10000) {
		cpu->in_mhz = 1;
		cpu->max_speed *= 1000;
		cpu->min_speed *= 1000;
		cpu->current_speed *= 1000;
	}

	if ((cpu->fd = open("/proc/stat", O_RDONLY)) < 0) {
		err = errno;
		perror("Nao pode abrir /proc/stat");
		return err;
	}

	if ((err = get_stat(cpu)) < 0) {
		perror("Nao pode ler /proc/stat");
		return err;
	}

	return 0;
}

/*
 * Signal handler for SIGTERM/SIGINT... clean up after ourselves
 */
void terminate() {
	int ncpus, i;
	cpuinfo_t *cpu;
	
	ncpus = sysconf(_SC_NPROCESSORS_CONF);
	if (ncpus < 1) ncpus = 1;
        
	/* 
	 * for each cpu, force it back to full speed.
	 * don't mix this with the below statement.
	 * 
	 * 5 minutes ago I convinced myself you couldn't 
	 * mix these two, now I can't remember why.  
	 */
        
	if(selector_cpu == -1) {
		for(i = 0; i < ncpus; i++) {
			cpu = all_cpus[i];
			func = LEAPS;
			change_speed(cpu, LOWER);
		}
	} else {
			cpu = all_cpus[selector_cpu];
			func = LEAPS;
			change_speed(cpu, LOWER);
	}

	for(i = 0; i < ncpus; i++) {
		cpu = all_cpus[i];

		close(cpu->fd);

		free(cpu->sysfs_dir);
		free(cpu->last_reading);
		free(cpu->reading);
		free(cpu->freq_table);
		free(cpu);
	}
	free(all_cpus);
	time_t duration = time(NULL) - start_time;
	pprintf(1,"Estatisticas:\n");
	pprintf(1,"  %d muda a velocidade em %d segundos\n",
			change_speed_count, (unsigned int) duration);
	pprintf(0,"FreqChange, Saindo.\n");
	closelog();

	exit(0);
}

const char *str_func(void) {
	switch (func) {
		case SINE: return "SINE";
		case AGGRESSIVE: return "AGGRESSIVE";
		case PASSIVE: return "PASSIVE";
		case LEAPS: return "LEAPS";
		default: return "UNKNOWN";
	}
}

// cpuid function lifted from kernel sources 
static inline void cpuid(int op, int *eax, int *ebx, int *ecx, int *edx) {
        __asm__("cpuid"
                : "=a" (*eax),
                  "=b" (*ebx),
                  "=c" (*ecx),
                  "=d" (*edx)
                : "0" (op));
}

/* 
 * A little bit of black magic to try and detect the number of cores per
 * processor.  This will have to be added on to for every architecture, 
 * as we learn how to detect them from userspace.  Note: This method
 * assumes uniform processor/thread ID's.  First look for affected_cpus, 
 * if not, then fall back to the cpuid check, or just default to 1. 
 *
 * By 'thread' in this case, I mean 'entity that is part of one scalable
 * instance'.  For example, a P4 with hyperthreading has 2 threads in 
 * one scalable instace.  So does an Athlon X2 dual core, because each
 * core has to have the same speed.  The new Yonah, on the other hand, may
 * have two scalable elements, as rumors say you can control both cores' 
 * speed individually.  Lets hope the speedstep driver populates affected_cpus
 * correctly...
 *
 * You can always override this by using the -c command line option to 
 * specify the number of threads per core.  If you do so, it will do a static
 * mapping, uniform for all real processors in the system.  Actually, so 
 * will this one, because there's no way for me to bind to a processor.
 * (yet. :)
 */
 
/* 
 * Main program loop.. parse arguments, sanity chacks, setup signal handlers
 * and then enter main loop
 */
int main(int argc, char **argv) {

	cpuinfo_t *cpu;
	int ncpus, mostraUso = 0, i, j, err, num_real_cpus, threads_per_core, cpubase;
	enum modes change, change2;
		int c;
	while(1) {
		c = getopt(argc, argv, "dnvhu:m:s:c:e:b:");
		if (c == -1)
			break;
		switch(c) {
			case 'u':
				selector_cpu = strtol(optarg, NULL, 10);
				if(selector_cpu < -1) {
					printf("Numero especificado para a cpu incompativel\n");
					help();
					exit(ENOTSUP);
				}
				break;
			case 'd':
				daemonize = 0;
				break;
			case 'n':
				ignore_nice = 0;
				break;
 			case 'v':
 				verbosity++;
				if (verbosity > 10) verbosity = 10;
 				break;
 			case 'c':
				t_per_core = strtol(optarg, NULL, 10);
				if (t_per_core < 1) {
					printf("Invalido numero de cores/proc\n");
					help();
					exit(ENOTSUP);
				}
				cores_specified = 1;
				break;
			case 'm':
				func = strtol(optarg, NULL, 10);
				if ((func < 0) || (func > 3)) {
					printf("Modo especifico ivalido\n");
					help();
					exit(ENOTSUP);
				}
				pprintf(2,"Usando o modo %s.\n", str_func());
				break;
			case 's':
				step = strtol(optarg, NULL, 10);
				if (step < 0) {
					printf("Etapa nao deve ser negativa");
					help();
					exit(ENOTSUP);
				}
				step_specified = 1;
				pprintf(2,"Usando a etapa %dHz.\n", step);
				break;
			case 'e':
				highwater = strtol(optarg, NULL, 10);
				if ((highwater < 0) || (highwater > 100)) {
					printf("Limite superior deve estar entre [0...100]\n");
					help();
					exit(ENOTSUP);
				}
				pprintf(2,"Usando o pct alto de %d%%\n",highwater);
				break;
			case 'b':
				lowwater = strtol(optarg, NULL, 10);
				if ((lowwater < 0) || (lowwater > 100)) {
					printf("Limite inferior deve estar entre [0...100]\n");
					help();
					exit(ENOTSUP);
				}
				pprintf(2,"Usando o pct baixo de %d%%\n",lowwater);
				break;
			case 'h':
			default:
			help();
			return 0;
		}
	}
	/* so we don't interfere with anything, including ourself */
	nice(5);

	if (daemonize)
		openlog("freqchange", LOG_AUTHPRIV|LOG_PERROR, LOG_DAEMON);

	/* My ego's pretty big... */
	/*testa usuario raiz */
	if (getuid()) {
		printf("Somente o usuario raiz pode executar essa operacao.\n");
		exit(EPERM);
	}

	ncpus = sysconf(_SC_NPROCESSORS_CONF);
	if (ncpus < 0) {
		perror("sysconf nao pode determinar o numero de cpus, assumindo 1\n");
		ncpus = 1;
	}
	if(selector_cpu > ncpus) {
		printf("Aviso, identificador da cpu nao pode ser maior que: %d\n",ncpus);
		exit (1);
	}

	pprintf(1,"Definicoes:\n");
	pprintf(1,"  Verbosidade:     %4d\n", verbosity);
	pprintf(1,"  Modo:          %4d     (%s)\n", func, str_func());
	pprintf(1,"  Etapa:          %4d MHz (%d kHz)\n", step/1000, step);
	pprintf(1,"  Intervalo: %4d ms\n", poll);

	/* 
	 * This should tell us the number of CPUs that Linux thinks we have,
	 * or, at least GLIBC
	 */
	if (cores_specified) {
		if (ncpus < t_per_core) {
			printf("AVISO: numero de threads por core falso, assumindo 1\n");
			threads_per_core = 1;
		} else {
			threads_per_core = t_per_core;
		}
	} else
		threads_per_core = 1;
	
	/* We don't support mixed configs yet */
	if (!ncpus || !threads_per_core || ncpus % threads_per_core) {	
		printf("WARN: ncpus(%d) is not a multiple of threads_per_core(%d)!\n",
			ncpus, threads_per_core);
		printf("AVISO: assumindo 1.\n");
		threads_per_core = 1;
		help();
		exit(ENOTSUP);
	}
	num_real_cpus = ncpus/threads_per_core;

	/* Malloc, inicializa a estrutura de dados */
	all_cpus = (cpuinfo_t **) malloc(sizeof(cpuinfo_t *)*ncpus);
	if (all_cpus == (cpuinfo_t **)NULL) {
		perror("Nao foi possivel alocar memoria para all_cpus");
		return ENOMEM;
	}

	for (i=0; i<ncpus; i++) {
		all_cpus[i] = (cpuinfo_t *)malloc(sizeof(cpuinfo_t));
		if (all_cpus[i] == (cpuinfo_t *)NULL) {
			perror("Nao foi possivel alocar memoria para all_cpus");
			return ENOMEM;
		}
		memset(all_cpus[i],0,sizeof(cpuinfo_t));
	}

	for (i=0;i<ncpus;i++) {
		all_cpus[i]->threads_per_core = threads_per_core;
		all_cpus[i]->scalable_unit = (i/threads_per_core)*threads_per_core;
	}

	pprintf(0,"Encontrado %d unidade%c escalave%s:  -- %d 'CPU%c' por unidade escalavel\n",
			num_real_cpus,
			(num_real_cpus>1)?'s':' ',(num_real_cpus>1)?"is":"l",
			threads_per_core,
			(threads_per_core>1)?'s':' ');

	for (i=0 ; i<ncpus ; i++) {
		cpu = all_cpus[i];
		if ((err = get_per_cpu_info(cpu, i)) != 0) {
			printf("\n");
			goto out;
		}
		pprintf(0,"  cpu%d: %dMhz - %dMhz (%d etapas)\n", 
				cpu->cpuid,
				cpu->min_speed / 1000, 
				cpu->max_speed / 1000, 
				cpu->table_size);
		for(j=0 ; j<cpu->table_size ; j++) {
			pprintf(1, "    etapa %d : %ldMhz -> %ld%%\n", j+1, 
					cpu->freq_table[j] / 1000,((cpu->freq_table[j]*100) / cpu->freq_table[0]));
		}
	}
	/* now that everything's all set up, lets set up a exit handler */
	//signal(SIGTERM, terminate);
	//signal(SIGINT, terminate);

	if (daemonize)
		daemon(0, 0);

	start_time = time(NULL);
	int counter = ncpus;
        
	while(counter > 0) {
		if(selector_cpu == -1) {
			usleep(poll*1000);
			for(i=0; i<num_real_cpus; i++) {
				change = LOWER;
				cpubase = i;//*threads_per_core;
				pprintf(6, "cpuid = %d, cpubase = %d, ",i,cpubase);
				all_cpus[cpubase]->current_level = 
					((all_cpus[cpubase]->current_speed*100) / all_cpus[cpubase]->max_speed);
				/* handle SMT/CMP here */
				for (j=0; j<all_cpus[cpubase]->threads_per_core; j++) {
					change2 = decide_speed(all_cpus[cpubase+j]);
					pprintf(6, "change = %d, change2 = %d\n",change,change2);

					if (change2 > change)
						change = change2;
				}
				if (change != SAME)
					change_speed(all_cpus[cpubase], change);
			}

		} else {
			usleep(poll*1000);
			change = LOWER;
			cpubase = selector_cpu*threads_per_core;
			pprintf(6, "cpuid = %d, cpubase = %d, ",selector_cpu,cpubase);
			all_cpus[cpubase]->current_level = ((all_cpus[cpubase]->current_speed*100) / 
				all_cpus[cpubase]->max_speed);
			/* handle SMT/CMP here */
			for(j=0 ; j<all_cpus[cpubase]->threads_per_core ; j++) {
				change2 = decide_speed(all_cpus[cpubase]);
				pprintf(6, "change = %d, change2 = %d\n",change,change2);

				if (change2 > change)
					change = change2;
			}
			/* here we set up frequency with the mode
                        already established */
			if (change != SAME)
				change_speed(all_cpus[cpubase], change);
		}
		counter--;
	}
        // now, we can stop the program with all the frequencies established
	terminate();
	/* should free more here.. will get to that later.... */
	/* or we can just be lazy and let the OS do it for us... */
out:
	printf("Problemas?\n");
	printf(" - Voce deve rodar em um kernel v.2.6.7 ou superior\n");
	printf(" - sysfs deve estar montado em /sys\n");
	printf(" - E preciso ter base para core cpufreq e cpufreq-userspace\n");
	printf(" - E necessario ter o cpufreq driver para sua cpu ser lida,\n");

	free(cpu);
	return err;
}
