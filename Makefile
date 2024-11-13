# Environment
ENV_FILES    = .env
#Compiler and Linker
CC          := gcc

#The Target Binary Program
TARGET      := app

#The Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := src
INCDIR      := include
BUILDDIR    := obj
TARGETDIR   := bin
RESDIR      := resources
SRCEXT      := c
DEPEXT      := d
OBJEXT      := o

#Flags, Libraries and Includes
CFLAGS      := -fopenmp -Wall -g
LIB         := -lm -lpq -lcurl
INC         := -I$(INCDIR)
INCDEP      := -I$(INCDIR)
# Mongoose build options. See https://mongoose.ws/documentation/#build-options
CFLAGS_MONGOOSE += -DMG_ENABLE_LINES=1
CFLAGS_EXTRA    ?= -DMG_TLS=MG_TLS_BUILTIN

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------
SOURCES     := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

include $(ENV_FILES)
export

#Default Make
all: resources $(TARGET)

run: resources $(TARGET)
	./$(TARGETDIR)/$(TARGET)

#Remake
remake: cleaner all

#Copy Resources from Resources Directory to Target Directory
resources: directories
	@cp -r $(RESDIR)/* $(TARGETDIR)/

#Make the Directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

#Clean only Objecst
clean:
	@$(RM) -rf $(BUILDDIR)

#Full Clean, Objects and Binaries
cleaner: clean
	@$(RM) -rf $(TARGETDIR)

#Pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

#Link
$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGETDIR)/$(TARGET) $^ $(LIB)

#Compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS_MONGOOSE) $(CFLAGS_EXTRA) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(CFLAGS_MONGOOSE) $(CFLAGS_EXTRA) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -fr $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -fr $(BUILDDIR)/$*.$(DEPEXT).tmp

#Non-File Targets
.PHONY: all remake clean cleaner resources
