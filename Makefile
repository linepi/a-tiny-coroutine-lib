COLIB_NAME = tinyco
COLIB_FILE = lib$(COLIB_NAME).so
TEST_TARGET = main

SRCS   := $(shell find . -maxdepth 1 -name "*.c")
HEADERS := $(shell find . -maxdepth 1 -name "*.h") $(SRCS)
CFLAGS += -U_FORTIFY_SOURCE
CFLAGS += -O2 -std=gnu11 -Wall -Wno-unused-result -Wno-unused-value -Wno-unused-variable -Wno-unused-function

all: $(COLIB_FILE)

$(COLIB_FILE): $(HEADERS) 
	gcc -fPIC -shared $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS)

TESTS = $(shell find tests -name "*.c")
TEST_TARGETS = $(addprefix tests/test-, $(notdir $(basename $(TESTS))))

test: $(TEST_TARGETS)
	@echo "======================= TEST START ========================"
	@LD_LIBRARY_PATH=. tests/test-$(TEST_TARGET)

tests/test-%: tests/%.c $(COLIB_FILE)
	gcc -g -O0 -L. $< -o $@ -l$(COLIB_NAME)

clean:
	rm -f $(COLIB_FILE) $(TEST_TARGETS)