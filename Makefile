# At the moment, there is no header dependency tracking
# For changes in the header, make clean make

# 1. Variables - Makes it easy to change compilers or flags later
CXX = g++					# Define the compiler
CXXFLAGS = -Wall -std=c++17	# Define compiler flags (e.g., enable warnings and use C++17)
LIBS = -lSDL2 -lGL
TARGET = main				# The final executable name
SRC = main.cpp Chip8.cpp Platform.cpp
OBJ = $(SRC:.cpp=.o)		# This reads in the values in SRC, replaces all .cpp extensions with .o

# 2. The "all" target - This is what runs by default
all: $(TARGET)

# 3. Linking - Combines object files into the final executable
# Make files follow the structure target: dependencies
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(LIBS)

# 4. Compilation - Turns .cpp files into .o (object) files
# The % is a wildcard that matches any filename
# $< and $@ are automatic variables
# $< will match the first dependency (the only one in this case)
# $@ will match the target
# Technically, the Linking block could replace $(TARGET) and $(OBJ) with $@ and $<
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 5. Clean - Removes the build files to start fresh
clean:
	rm -f $(OBJ) $(TARGET)