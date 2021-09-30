INCDIR = include
SRCDIR = src
OBJDIR = obj

# Compiler stuff
CC       = gcc
CXX      = g++
CPPFLAGS = -I $(INCDIR)
CFLAGS   = -g
LIBS     = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm

# List of object files
CHUNKS = $(OBJDIR)/cmbChunk.o $(OBJDIR)/sklChunk.o $(OBJDIR)/matsChunk.o $(OBJDIR)/texChunk.o $(OBJDIR)/sklmChunk.o $(OBJDIR)/lutsChunk.o $(OBJDIR)/vatrChunk.o
CMB    = $(OBJDIR)/cmb.o $(OBJDIR)/cmbTextures.o $(CHUNKS)
OGL    = $(OBJDIR)/camera.o $(OBJDIR)/cmbShader.o $(OBJDIR)/cmbModel.o $(OBJDIR)/glad.o

# Idk what this is for
.PHONY: all clean
all: ViewCMB ReadCMB

# Rule for C files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Rule for C++ files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

ViewCMB: $(OBJDIR)/viewCMB.o $(OGL) $(CMB)
	$(CXX) -o $@.out $^ $(LIBS)

ReadCMB: $(OBJDIR)/readCMB.o $(CMB)
	$(CC) -o $@.out $^

ReadZAR: $(OBJDIR)/readZAR.o
	$(CC) $(CFLAGS).out -o $@ $^

ReadZSI: $(OBJDIR)/readZSI.o
	$(CC) $(CFLAGS).out -o $@ $^

ReadCTXB: $(OBJDIR)/readCTXB.o
	$(CC) $(CFLAGS).out -o $@ $^

Tags:
	ctags -R * /usr/include/GLFW/* /usr/include/glad/*

clean:
	rm -f $(OBJDIR)/*.o ReadCMB.out ViewCMB.out Zar.out
