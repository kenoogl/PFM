CXX ?= clang++
CXXFLAGS ?= -std=c++17 -O2

COMMON_SRCS = wingxa.cpp

.PHONY: all clean

all: three_phase_decomp plot bplot

three_phase_decomp: three_phase_decomp.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

plot: plot.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

bplot: bplot.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f three_phase_decomp plot bplot c_field_*.bmp test.dat
	rm -rf output
