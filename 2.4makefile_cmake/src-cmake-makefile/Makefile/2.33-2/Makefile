.PHONY: all clean
MKDIR = mkdir
RM = rm
RMFLAGS = -fr
CC = gcc

DIR_OBJS = objs
DIR_EXES = exes
DIR_DEPS = deps
DIRS = $(DIR_OBJS) $(DIR_EXES) $(DIR_DEPS)

EXE = complicated
EXE := $(addprefix $(DIR_EXES)/, $(EXE))
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
OBJS := $(addprefix $(DIR_OBJS)/, $(OBJS))
DEPS = $(SRCS:.c=.dep)
DEPS := $(addprefix $(DIR_DEPS)/, $(DEPS))

all: $(EXE)

ifeq ("$(wildcard $(DIR_DEPS))", "")
     DEP_DIR_DEPS:=$(DIR_DEPS)
endif

-include $(DEPS)

$(DIRS):
	$(MKDIR) $@

$(EXE): $(DIR_EXES) $(OBJS)
	$(CC) -o $@ $(filter %.o, $^)

$(DIR_OBJS)/%.o: $(DIR_OBJS) %.c
	$(CC) -o $@ -c $(filter %.c, $^)

$(DIR_DEPS)/%.dep: $(DEP_DIR_DEPS) %.c
	@echo "Making $@ ..."
	@set -e; \
	$(RM) $(RMFLAGS) $@.tmp ; \
	$(CC) -E -MM $(filter %.c, $^) > $@.tmp ; \
	sed 's,\(.*\)\.o[ :]*,objs/\1.o: ,g' < $@.tmp > $@ ; \
	$(RM) $(RMFLAGS) $@.tmp

clean:
	$(RM) $(RMFLAGS) $(DIRS)
