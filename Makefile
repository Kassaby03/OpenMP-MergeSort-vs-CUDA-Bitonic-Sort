CXX = g++
CXXFLAGS = -O3 -Wall -Iinclude

SRCDIR = src
BINDIR = bin
OBJDIR = obj

SRCS = $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(SRCDIR)/sort/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
EXEC = $(BINDIR)/sort_app

all: $(EXEC)

$(EXEC): $(OBJS)
	@mkdir -p $(BINDIR) results
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)
