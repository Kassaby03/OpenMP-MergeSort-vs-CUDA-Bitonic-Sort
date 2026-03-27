CXX = g++
NVCC := $(shell command -v nvcc 2>/dev/null)
CUDA_HOME := $(if $(NVCC),$(shell dirname $(dir $(NVCC))))

CXXFLAGS = -O3 -Wall -fopenmp -Iinclude
NVCCFLAGS = -O3 -Iinclude

ifneq ($(CUDA_ARCH),)
NVCCFLAGS += -arch=$(CUDA_ARCH)
endif

SRCDIR = src
BINDIR = bin
OBJDIR = obj

SRCS_CPP = $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(SRCDIR)/sort/*.cpp)
SRCS_CU = $(wildcard $(SRCDIR)/sort/*.cu)

OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS_CPP))

ifneq ($(NVCC),)
OBJS += $(patsubst $(SRCDIR)/%.cu,$(OBJDIR)/%.o,$(SRCS_CU))
CXXFLAGS += -DHAS_CUDA
ifneq ($(CUDA_HOME),)
CXXFLAGS += -I$(CUDA_HOME)/include
NVCCFLAGS += -I$(CUDA_HOME)/include
endif
endif

ifeq ($(NVCC),)
LINK_CMD = $(CXX) $(CXXFLAGS) -o $@ $(OBJS)
else
LINK_CMD = $(NVCC) $(NVCCFLAGS) -Xcompiler -fopenmp -o $@ $(OBJS)
endif

EXEC = $(BINDIR)/sort_app

all: $(EXEC)

$(EXEC): $(OBJS)
	@mkdir -p $(BINDIR) results
	$(LINK_CMD)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cu
	@mkdir -p $(dir $@)
	$(NVCC) $(NVCCFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)
