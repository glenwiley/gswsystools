/* shmtest.c
   Glen Wiley <glen.wiley@gmail.com>

   performance test for shared memory mechanisms

   TODO test for read
   TODO report limits for the process
   TODO test that interleaves read/write
   TODO complain about size overflow (max is currently 15GB)
   TODO add support for SHM_HUGETLB under linux
   TODO comma delimited output to feed spreadsheet
   TODO compare aligned/non-aligned access
*/

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

/*-------------------- vars and constants */

const char    *PRGNAME     = "shmtst";  /* used to talk to user */
int           g_verbose    = 0;         /* if 1 then print more diag msgs */
unsigned long g_blksz_sm   = 100UL;     /* block size, small */
unsigned long g_blksz_lg   = 10000UL;   /* block size, large */
long          g_tstlength  = 5000000L;  /* 5 second test inteval */
long          g_timechkops = 500000L;   /* # ops between time tests */

/*-------------------- functions */

void usage(const char *prg);
unsigned long getsize(char *str);
void *makesysvshm(const char *keyfn, int keypj, unsigned long sz);
void rmsysvshm(const char *keyfn, int keypj);
void tst_memwrt(char *svcnm, void *addr, unsigned long sz);
void tst_memrd(char *svcnm, void *addr, unsigned long sz);
void *mmapanon(unsigned long sz);

/*---------------------------------------- main */
int
main(int argc, char *argv[])
{
   int  opt;
   int           cfg_iter  = 5;
   unsigned long cfg_shmsz = 0UL;
   int           cfg_proj  = 0;
   char          *cfg_keyfn;
   void          *addr = NULL;

   cfg_keyfn = argv[0];

   while((opt = getopt(argc, argv, "?i:k:p:s:v")) != -1)
   {
      switch(opt)
      {
         case 'i':
            cfg_iter = atoi(optarg);
            break;

         case 'k':
            cfg_keyfn = optarg;
            break;

         case 'p':
            cfg_proj = atoi(optarg);
            break;

         case 's':
            cfg_shmsz = getsize(optarg);
            break;

         case 'v':
            g_verbose = 1;
            break;

         case '?':
         case 'h':
         default:
            usage(PRGNAME);
            exit(1);
      }
   } /* while getopt */

   /* validate the configuration before we roll and exit or at least
      complain about any unreasonable config values */

   if(cfg_shmsz <= 0UL)
   {
      fprintf(stderr, "%s: error, shared memory segment size is 0, specify using '-s <size>' with suffix of b/k/m/g\n", PRGNAME);
      exit(1);
   }

   if(cfg_iter < 0)
   {
      fprintf(stderr, "%s: error, number of iterations < 0\n", PRGNAME);
      exit(1);
   }

   if(g_verbose)
   {
      printf("%s: shm key file=%s\n",    PRGNAME, cfg_keyfn);
      printf("%s: shm key project=%d\n", PRGNAME, cfg_proj);
      printf("%s: shm size=%lu bytes\n", PRGNAME, cfg_shmsz);
      printf("%s: block size, small=%lu bytes\n", PRGNAME, g_blksz_sm);
      printf("%s: block size, large=%lu bytes\n", PRGNAME, g_blksz_lg);
   }

   printf("%-7s %-11s %12s %12s %12s\n"
    , "Service", "Test", "Num Ops", "Time(usec)", "Ops/Sec");

   /* iterate through the full set of tests for the number of
      times specified */

   for(; cfg_iter != 0; cfg_iter--)
   {
      /*-------------------- heap tests */

      addr = (void *) malloc(cfg_shmsz);
      if(addr != NULL)
      {
         tst_memwrt("heap", addr, cfg_shmsz);
         tst_memrd("heap", addr, cfg_shmsz);

         free(addr);
         addr = NULL;
      }
      else
         fprintf(stderr, "%s: error in malloc(), %d, %s\n"
          , PRGNAME, errno, strerror(errno));

      /*-------------------- system V shared memory tests */

      addr = makesysvshm(cfg_keyfn, cfg_proj, cfg_shmsz);
      if(addr != NULL)
      {
         tst_memwrt("sysvshm", addr, cfg_shmsz);
         tst_memrd("sysvshm", addr, cfg_shmsz);

         shmdt(addr);
         rmsysvshm(cfg_keyfn, cfg_proj);
      }

      /*-------------------- mmap tests */

      addr = mmapanon(cfg_shmsz);
      if(addr != NULL)
      {
         tst_memwrt("mmapann", addr, cfg_shmsz);
         tst_memrd("mmapann", addr, cfg_shmsz);

         munmap(addr, cfg_shmsz);
      }
   } /* for cfg_iter */

   return 0;
} /* main */

/*---------------------------------------- mmapanon
*/
void *
mmapanon(unsigned long sz)
{
   void *addr = NULL;

#ifdef sun
   addr = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON
    , -1, 0);
#endif
#ifdef linux
   addr = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS
    , 0, 0);
#endif
   if(addr == (void *) -1)
   {
      fprintf(stderr, "%s: error in mmap(), %d, %s\n"
       , PRGNAME, errno, strerror(errno));
      addr = NULL;
   }
      
   return addr;
} /* mmapanon */

/*---------------------------------------- makesysvshm
  keyfn = filename to used to generate key
  keypj = project number used to generate key

  TODO if already segment exists, remove it

  returns NULL on error, memory address on success
*/
void *
makesysvshm(const char *keyfn, int keypj, unsigned long sz)
{
   key_t shmk;
   int   shmid = -1;
   void  *addr = NULL;

   shmk = ftok(keyfn, keypj);
   if(shmk == -1)
   {
      fprintf(stderr, "%s: error in ftok(), %d, %s\n", PRGNAME, errno, strerror(errno));
   }
   else
   {
      rmsysvshm(keyfn, keypj);

      /* now actually create the shared memory segment */

      shmid = shmget(shmk, sz, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
      if(shmid == -1)
      {
         fprintf(stderr, "%s: error in shmget(), %d, %s\n"
          , PRGNAME, errno, strerror(errno));
      }
      else
      {
         addr = shmat(shmid, NULL, 0);
         if(addr == (void *) -1)
         {
            fprintf(stderr, "%s: error in shmat(), %d, %s\n"
             , PRGNAME, errno, strerror(errno));
            addr = NULL;
         }
      }
   }

   return addr;
} /* makesysvshm */

/*---------------------------------------- rmsysvshm
*/
void
rmsysvshm(const char *keyfn, int keypj)
{
   key_t shmk;
   int   shmid;

   shmk = ftok(keyfn, keypj);
   if(shmk == -1)
   {
      fprintf(stderr, "%s: error in ftok(), %d, %s\n"
       , PRGNAME, errno, strerror(errno));
   }
   else
   {
      /* remove the shared memory segment if it exists */

      shmid = shmget(shmk, 0, 0);
      if(shmid > -1)
      {
         if(g_verbose)
            printf("%s: removing existing system V shared memory segment\n"
             , PRGNAME);

         if(shmctl(shmid, IPC_RMID, NULL) == -1)
         {
            fprintf(stderr, "%s: error in shmctl(shmid, IPC_RMID), %d, %s\n"
             , PRGNAME, errno, strerror(errno));
         }
      }
   }

} /* rmsysvshm */

/*---------------------------------------- usage
*/
void
usage(const char *prg)
{
   printf(
    "USAGE: %s [-v] [-i <niter>] [-k <file>] [-p <proj>] -s <size>\n"
    "\n"
    "  -i <niter>  number of iterations, default=5\n"
    "  -k <file>   file name used to generate shm key, default=this executable\n"
    "  -p <proj>   project number used to generate shm key, default=0\n"
    "  -s <size>   shared memory size in bytes\n"
    "              (specify K, M, G for kilo, mega, giga bytes)\n"
    "\n"
    "  -v          verbose, display more diagnostic messages\n"
    "\n"
    , prg);

} /* usage */

/*---------------------------------------- getsize
  convert string to bytes using a suffix of b, k, m, g
  if an unrecognized suffix is specified then return size of 0
*/
unsigned long
getsize(char *str)
{
   char m;
   unsigned long sz = 0UL;

   sscanf(str, "%lu%c", &sz, &m);
   m = tolower(m);
   switch(m)
   {
      case'\0':
      case 'b':
         sz = sz;
         break;
      case 'k':
         sz = sz * 1024;
         break;
      case 'm':
         sz = sz * 1024 * 1024;
         break;
      case 'g':
         sz = sz * 1024 * 1024 * 1024;
         break;
      default:
         sz = 0UL;
   }

   return sz;
} /* getsize */

/*---------------------------------------- tst_memwrt
  test write performance

   multiple small writes to concurrent locations
   TODO
   multiple large writes to concurrent locations
   multiple small writes to widely separated locations
   multiple large writes to widely separated locations
*/
void
tst_memwrt(char *svcnm, void *tgtaddr, unsigned long sz)
{
   struct timeval tvbeg;
   struct timeval tvend;
   unsigned long  nops;
   long           intvl;
   long           nopscheck;
   char           *buf[2];
   int            bufn = 0;
   void           *addr;

   addr = tgtaddr;

   /* create buffers that will be copied into the shared memory area,
      we use 2 to try to ensure that each write changes the memory */

   buf[0] = (char *) malloc(g_blksz_sm);
   buf[1] = (char *) malloc(g_blksz_sm);
   if(buf[0] == NULL || buf[1] == NULL)
   {
      fprintf(stderr, "%s: error in malloc(), %d, %s\n"
       , PRGNAME, errno, strerror(errno));
      exit(1);
   }
   memset(buf[0], 'A', g_blksz_sm);
   memset(buf[1], 'B', g_blksz_sm);

   /*-------------------- small writes to concurrent locs */

   nops = 0L;
   nopscheck = 0L;
   gettimeofday(&tvbeg, NULL);
   while(1)
   {
      memcpy(addr, buf[bufn], g_blksz_sm);
      nops++;
      nopscheck++;

      /* every so many operations, check to see whether our test time
         has expired
         TODO this will hurt metrics for a very fast system, use alarm?
      */
      if(g_timechkops < nopscheck)
      {
         nopscheck = 0L;
         gettimeofday(&tvend, NULL);
         intvl = ((tvend.tv_sec - tvbeg.tv_sec) * 1000000) 
          + (tvend.tv_usec - tvbeg.tv_usec);
         if(intvl >= g_tstlength)
            break;
      }

      /* increment our target address and make sure we wrap back to
         the beginning once we reach the end of the allocated space */

      addr = addr + g_blksz_sm;
      if( addr > (tgtaddr - g_blksz_sm))
      {
         /* toggle source buffer to ensure memory changes */
         bufn = (bufn == 0 ? 1 : 0);
         addr = tgtaddr;
      }
   } /* while(1) */

   printf("%-7s %-11s %12lu %12lu %12.0lf\n"
    , svcnm, "wrt_sm_seq", nops, intvl, (nops * 1.0 /(intvl/100000.0)));

   if(buf[0] != NULL)
      free(buf[0]);
   if(buf[1] != NULL)
      free(buf[1]);

   return;
} /* tst_memwrt */

/*---------------------------------------- tst_memrd
  test read performance

  assumes that the memory has been written earlier via tst_memwrt

  multiple small reads from concurrent locations
*/
void
tst_memrd(char *svcnm, void *tgtaddr, unsigned long sz)
{
   struct timeval tvbeg;
   struct timeval tvend;
   unsigned long  nops;
   long           intvl;
   long           nopscheck;
   char           *buf;
   void           *addr;

   addr = tgtaddr;

   /* create buffer that will be copied from the shared memory area */

   buf = (char *) malloc(g_blksz_sm);
   if(buf == NULL)
   {
      fprintf(stderr, "%s: error in malloc(), %d, %s\n"
       , PRGNAME, errno, strerror(errno));
      exit(1);
   }

   /*-------------------- small reads from concurrent locs */

   nops = 0L;
   nopscheck = 0L;
   gettimeofday(&tvbeg, NULL);
   while(1)
   {
      memcpy(buf, addr, g_blksz_sm);
      nops++;
      nopscheck++;

      /* every so many operations, check to see whether our test time
         has expired
         TODO this will hurt metrics for a very fast system, use alarm?
      */
      if(g_timechkops < nopscheck)
      {
         nopscheck = 0L;
         gettimeofday(&tvend, NULL);
         intvl = ((tvend.tv_sec - tvbeg.tv_sec) * 1000000) 
          + (tvend.tv_usec - tvbeg.tv_usec);
         if(intvl >= g_tstlength)
            break;
      }

      /* increment our target address and make sure we wrap back to
         the beginning once we reach the end of the allocated space */

      addr = addr + g_blksz_sm;
      if( addr > (tgtaddr - g_blksz_sm))
         addr = tgtaddr;
   } /* while(1) */

   printf("%-7s %-11s %12lu %12lu %12.0lf\n"
    , svcnm, "rd_sm_seq", nops, intvl, (nops * 1.0 /(intvl/100000.0)));

   if(buf != NULL)
      free(buf);

   return;
} /* tst_memrd */

/* shmtest.c */
