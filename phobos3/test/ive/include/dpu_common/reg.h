#ifdef STDOUT_PTF
#define CVI_LOG(...) printf(__VA_ARGS__), printf("\n")
#else
#define CVI_LOG log_debug
#endif

#define REG_TYPE_IS_REG 0
#define REG_TYPE_IS_LUT 1
#define DUMP_ALL   0
#define DUMP_HW    1
#define DUMP_SW    2
#define DUMP_TBD_FILE "temp.ini" 
#define BUF_SIZE  256 

#define STR(name) #name
#define STR_FUNC(name) STR(name)
#define MODULE_INCLUDE_DEF() STR_FUNC(MODULE()REG_DEF_H())
#define PASTER(x,y) x ## _ ## y
#define REG_ST_DEF(regname, regtype, regdepth) \
  regtype regname:regdepth;\
struct PASTER(_reg_st,regname) { \
  regtype value:regdepth;\
  char name[BUF_SIZE];\
  int  type;\
  int  count;\
} PASTER(regname,info);

#define LUT_ST_DEF(lutname, luttype, lutsize) \
  luttype lutname[lutsize];\
struct PASTER(_lut_st,lutname) { \
  char name[BUF_SIZE];\
  int  type;\
  int  size;\
  int  count;\
} PASTER(lutname,info);

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
#define LOAD_REG(REG_MODE, regname) \
  if(MATCH(REG_MODE->section, STR(regname))) {\
    REG_MODE->regname = atoi(value);\
    REG_MODE->PASTER(regname,info).count = 1;\
    return 1;\
  }

#define LOAD_LUT(REG_MODE, lutname) \
  if(MATCH(REG_MODE->section, STR(lutname))) {\
    if (tok_idx == 0) { \
      REG_MODE->PASTER(lutname,info).count = 1; \
    } \
    tok = strtok((char *)value, ","); \
    REG_MODE->lutname[tok_idx] = atoi(tok); \
    while(tok!=NULL) { \
      tok_idx++; \
      tok = strtok(NULL, " ,"); \
      if(tok!=NULL) \
        REG_MODE->lutname[tok_idx] = atoi(tok); \
        REG_MODE->PASTER(lutname,info).count = tok_idx; \
    } \
    if (tok_idx == REG_MODE->PASTER(lutname,info).size) {\
      return 1;\
    } \
  }

#define LOAD_SECTION(name) 

#define DUMP_SECTION(REG_MODE)      fprintf(fp, "[%s]\n", REG_MODE->section);
#define DUMP_REG(REG_MODE, regname , depth) \
  if(atoi(STR(depth))>32)\
    fprintf(fp, "%s = %ld\n", REG_MODE->PASTER(regname,info).name, REG_MODE->regname);\
  else\
    fprintf(fp, "%s = %d\n", REG_MODE->PASTER(regname,info).name, REG_MODE->regname);
#define DUMP_LUT(REG_MODE, lutname) fprintf(fp, "%s = ", REG_MODE->PASTER(lutname,info).name); \
  for(i=0; i<REG_MODE->PASTER(lutname,info).size; i++) \
  {\
    if(i%8==0 && i!=0)\
      fprintf(fp, "\n");\
    fprintf(fp, "\t%d,", REG_MODE->lutname[i]);\
  }\
  fprintf(fp, "\n");


#define PRINT_SECTION(REG_MODE)      CVI_LOG("[%s]", REG_MODE->section);
#define PRINT_REG(REG_MODE, regname, depth) \
  if(atoi(STR(depth))>32)\
    CVI_LOG("%s.%s.%s = %0ld", REG_MODE->section, STR(REG_MODE), REG_MODE->PASTER(regname,info).name, REG_MODE->regname); \
  else \
    CVI_LOG("%s.%s.%s = %0d", REG_MODE->section, STR(REG_MODE), REG_MODE->PASTER(regname,info).name, REG_MODE->regname);
  //CVI_LOG(comment);

#define PRINT_LUT(REG_MODE, lutname) \
  memset(array,'\0',sizeof(array));\
  CVI_LOG("%s.%s.%s = ", REG_MODE->section, STR(REG_MODE), REG_MODE->PASTER(lutname,info).name); \
  for(i=0; i<REG_MODE->PASTER(lutname,info).size; i++) \
  {\
    if(i%8==0 && i!=0){\
      CVI_LOG("%s", array); \
      memset(array,'\0',sizeof(array));\
    }\
    sprintf(array, "%s \t%d,", array, REG_MODE->lutname[i]); \
    if(i==REG_MODE->PASTER(lutname,info).size-1)\
      CVI_LOG("%s", array);\
  }

#define INIT_SECTION(REG_MODE, name) strcpy(REG_MODE->section, STR(name));
#define INIT_REG(REG_MODE, regname, reginit) strcpy(REG_MODE->PASTER(regname,info).name, STR(regname)); REG_MODE->PASTER(regname,info).type=REG_TYPE_IS_REG; REG_MODE->PASTER(regname,info).count=0; REG_MODE->regname=reginit;
#define INIT_LUT(REG_MODE, lutname, lutsize)     strcpy(REG_MODE->PASTER(lutname,info).name, STR(lutname)); REG_MODE->PASTER(lutname,info).type=REG_TYPE_IS_LUT; REG_MODE->PASTER(lutname,info).size=lutsize; REG_MODE->PASTER(lutname,info).count=0;

/*
#ifdef CVI_REG_DEBUG
#define LOAD_INI_CHECK_REG_DEF(REG_MODE, regname, ...) \
        if(inst.REG_MODE.PASTER(regname,info).count != 1) { \
          CVI_LOG("WARNING! %s.%s.%s is not set in .ini file", inst.REG_MODE.section, STR(REG_MODE), inst.REG_MODE.PASTER(regname,info).name);\
	  getchar();\
        }
#define LOAD_INI_CHECK_LUT_DEF(REG_MODE, lutname, ...) \
        if(inst.REG_MODE.PASTER(lutname,info).count != inst.REG_MODE.PASTER(lutname,info).size) { \
          CVI_LOG("ERROR! %s.%s.%s is not full in .ini file, expected %0d, but actually %0d", inst.REG_MODE.section, STR(REG_MODE), inst.REG_MODE.PASTER(lutname,info).name, inst.REG_MODE.PASTER(lutname,info).size, inst.REG_MODE.PASTER(lutname,info).count);\
	  getchar();\
        }
#else
*/
#define LOAD_INI_CHECK_REG_DEF(REG_MODE, regname, ...) \
        if(inst.REG_MODE.PASTER(regname,info).count != 1) { \
          CVI_LOG("WARNING! %s.%s.%s is not set in .ini file", inst.REG_MODE.section, STR(REG_MODE), inst.REG_MODE.PASTER(regname,info).name);\
        }
#define LOAD_INI_CHECK_LUT_DEF(REG_MODE, lutname, ...) \
        if(inst.REG_MODE.PASTER(lutname,info).count != inst.REG_MODE.PASTER(lutname,info).size) { \
          CVI_LOG("ERROR! %s.%s.%s is not full in .ini file, expected %0d, but actually %0d", inst.REG_MODE.section, STR(REG_MODE), inst.REG_MODE.PASTER(lutname,info).name, inst.REG_MODE.PASTER(lutname,info).size, inst.REG_MODE.PASTER(lutname,info).count);\
        }
//#endif

#define GEN_DUMP_INI_BASE_FUNC(name, mode, reg_dump_ini) \
  int name(char *f) \
  {\
    /*copy original file*/ \
    FILE *fp_read, *fp_write; \
    char * line = NULL;\
    size_t len = 0; \
    ssize_t read; \
    int hit_section = 0; \
    int hit_section_exist = 0; \
    regex_t preg; \
    const char* pattern = "^[[a-z0-9_]+]"; \
    \
    int success = regcomp(&preg, pattern, REG_EXTENDED|REG_ICASE);\
    assert(success==0);\
    regmatch_t matchptr[1];   /* 記錄匹配結果陣列，長度為1僅記錄 full match */\
    const size_t nmatch = 1;    /*  matchptr陣列長度 */\
    char buf[BUF_SIZE];\
    CVI_LOG("[%s] Dump INI configurations in %s...", STR(name), f);\
    \
    fp_read = fopen(f, "r");\
    if (fp_read == NULL) { /*file does not exist */\
      fp_write = fopen(f, "w+");\
      reg_dump_ini(fp_write, &inst.mode);\
      fclose(fp_write);\
    \
    } else {\
      fp_write = fopen(DUMP_TBD_FILE, "w+");\
      while ((read = getline(&line, &len, fp_read)) != -1) {\
        int status = regexec(&preg, line, nmatch, matchptr, 0);\
        if (status == REG_NOMATCH){\
          if (hit_section == 0) {\
            fprintf(fp_write, "%s", line);\
          }\
        }\
        else if (status == 0){  \
            sprintf(buf,"[%s]\n", &inst.mode.section);\
            if(strcmp(line, buf)==0) {\
      	    reg_dump_ini(fp_write, &inst.mode);\
              hit_section = 1;\
              hit_section_exist = 1;\
            } else {\
              fprintf(fp_write, "%s", line);\
              hit_section = 0;\
            }\
        }\
        else {  /* 執行錯誤 */\
            char msgbuf[BUF_SIZE];\
            regerror(status, &preg, msgbuf, sizeof(msgbuf)); \
            printf("error: %s\n", msgbuf);\
        }\
      }\
      regfree(&preg);\
      fclose(fp_read);\
      fclose(fp_write);\
      \
      fp_read  = fopen(DUMP_TBD_FILE, "r"); \
      fp_write = fopen(f, "w+");\
      if (fp_read == NULL) { /*file does not exist*/\
        CVI_LOG("error: no temp.ini file!");\
        return 0;\
      }\
      while ((read = getline(&line, &len, fp_read)) != -1) {\
        fprintf(fp_write, "%s", line);\
      }\
      if (hit_section_exist ==0) { \
        reg_dump_ini(fp_write, &inst.mode);\
      } \
      \
      fclose(fp_read);\
      fclose(fp_write);\
      \
      if (line)\
        free(line);\
      \
    }\
    CVI_LOG("[%s] Dump INI configurations in %s done", STR(name), f);\
    \
    return 0;\
  }

