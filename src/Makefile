
# The root directory of this project, relative to where this file is.
TOP_DIR = ".."

CXX = g++
CPPFLAGS += -I$(TOP_DIR)/include
CXXFLAGS += -g -Wall -Wextra -W -std=c++98 -pedantic
LDFLAGS += -lpthread
OBJS = $(patsubst %.cc,%.o,$(wildcard *.cc))

all : libwsd.a

libwsd.a : $(OBJS)
	$(AR) -ru $@ $^

clean:
	$(RM) -f *.o *~ *.out .*.d *.a

DEPENDS = $(patsubst %.cc,.%.d,$(wildcard *.cc))
include $(DEPENDS)

# ---------------------------------------------------------------------------
# Recipes

%.o : %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.%.d : %.cc
	set -e; \
	$(CXX) -MM $(CPPFLAGS) $< | \
		sed -n 'H;$${g;s!.*:\(.*\)!$*.o $@: \$$(wildcard\1)!;p}' > $@
