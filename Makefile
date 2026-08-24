# Makefile para la Fase 1 del proyecto

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Wpedantic -Iinclude

# Directorios
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

# Archivos fuente
CORE_SRCS = $(wildcard $(SRCDIR)/core/*.cpp)
PARSER_SRCS = $(wildcard $(SRCDIR)/parser/*.cpp)
STRATEGIES_SRCS = $(wildcard $(SRCDIR)/strategies/*.cpp)
SOLVERS_SRCS = $(wildcard $(SRCDIR)/solvers/*.cpp)
MAIN_SRC = $(SRCDIR)/main.cpp
TEST_SRCS = $(wildcard tests/*.cpp)

# Archivos objeto
CORE_OBJS = $(patsubst $(SRCDIR)/core/%.cpp, $(OBJDIR)/core/%.o, $(CORE_SRCS))
PARSER_OBJS = $(patsubst $(SRCDIR)/parser/%.cpp, $(OBJDIR)/parser/%.o, $(PARSER_SRCS))
STRATEGIES_OBJS = $(patsubst $(SRCDIR)/strategies/%.cpp, $(OBJDIR)/strategies/%.o, $(STRATEGIES_SRCS))
SOLVERS_OBJS = $(patsubst $(SRCDIR)/solvers/%.cpp, $(OBJDIR)/solvers/%.o, $(SOLVERS_SRCS))
MAIN_OBJ = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(MAIN_SRC))

OBJS = $(CORE_OBJS) $(PARSER_OBJS) $(STRATEGIES_OBJS) $(SOLVERS_OBJS) $(MAIN_OBJ)
TEST_OBJS = $(CORE_OBJS) $(PARSER_OBJS) $(STRATEGIES_OBJS) $(SOLVERS_OBJS) $(patsubst tests/%.cpp, $(OBJDIR)/tests/%.o, $(TEST_SRCS))

# Ejecutable
TARGET = $(BINDIR)/app
TEST_TARGET = $(BINDIR)/test_app
LDFLAGS = -lCbc -lOsiClp -lOsi -lClp -lCoinUtils

.PHONY: all test run clean directories

all: directories $(TARGET)

directories:
	@mkdir -p $(OBJDIR)/core
	@mkdir -p $(OBJDIR)/parser
	@mkdir -p $(OBJDIR)/strategies
	@mkdir -p $(OBJDIR)/solvers
	@mkdir -p $(OBJDIR)/tests
	@mkdir -p $(BINDIR)
	@mkdir -p results

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST_TARGET): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/tests/%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/core/%.o: $(SRCDIR)/core/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/parser/%.o: $(SRCDIR)/parser/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/strategies/%.o: $(SRCDIR)/strategies/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/solvers/%.o: $(SRCDIR)/solvers/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: directories $(TEST_TARGET)
	@echo "Ejecutando suite de tests automatizados"
	./$(TEST_TARGET)

run: all
	./$(TARGET)

clean:
	@rm -rf $(OBJDIR)
	@rm -rf $(BINDIR)
	@rm -f $(SRCDIR)/*.o $(SRCDIR)/*/*.o $(OBJDIR)/*.o $(OBJDIR)/*/*.o tests/*.o *.o
