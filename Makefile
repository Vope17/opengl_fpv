CXX = g++
CXXFLAGS = -Iinclude -Wall -std=c++11 -g
LDFLAGS = -lglfw -ldl -lGL

SRC = src/main.cpp src/glad.c
OBJ = $(SRC:.cpp=.o)
OBJ := $(OBJ:.c=.o)

TEST_SRC = src/test.cpp
TEST_OBJ = $(TEST_SRC:.cpp=.o)

DEP = $(OBJ:.o=.d)
TEST_DEP = $(TEST_OBJ:.o=.d)

.PHONY: all app test clean run

all: app

app: $(OBJ)
	$(CXX) $^ $(LDFLAGS) -o $@ && ./app

test: $(TEST_OBJ)
	$(CXX) $^ $(LDFLAGS) -o test

%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MT $(@:.d=.o) $< > $@

%.d: %.c
	$(CXX) $(CXXFLAGS) -MM -MT $(@:.d=.o) $< > $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEP)
-include $(TEST_DEP)

run: app
	./app

runtest: test
	./test

clean:
	rm -f $(OBJ) $(TEST_OBJ) $(DEP) $(TEST_DEP) app test

