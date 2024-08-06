.PHONY: output clean

BUILD := objects

program_SRC := $(wildcard source/*.cpp)
program_OBJ := $(patsubst source%.cpp,$(BUILD)%.o,$(program_SRC))

FLAGS := -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ \
         -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations \
		 -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported \
		 -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal \
		 -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 \
		 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd \
		 -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self \
		 -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo \
		 -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn \
		 -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override \
		 -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code \
		 -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix \
		 -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast \
		 -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation \
		 -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer \
		 -Wstack-usage=20000 -pie -fPIE -Werror=vla \
		 -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,$\
		 float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,$\
		 null,object-size,return,returns-nonnull-attribute,shift,$\
		 signed-integer-overflow,undefined,unreachable,vla-bound,vptr

output: $(BUILD) $(BUILD)/main.o $(program_OBJ)
	@g++ $(program_OBJ) $(BUILD)/main.o $(FLAGS) -o $(BUILD)/output

$(BUILD):
	@mkdir $(BUILD)

$(BUILD)/stack.o: source/stack.cpp
	@g++ $(FLAGS) -c source/stack.cpp -o $(BUILD)/stack.o

$(BUILD)/file_processing.o: source/file_processing.cpp
	@g++ $(FLAGS) -c source/file_processing.cpp -o $(BUILD)/file_processing.o

$(BUILD)/compare_doubles.o: source/compare_doubles.cpp
	@g++ $(FLAGS) -c source/compare_doubles.cpp -o $(BUILD)/compare_doubles.o

$(BUILD)/processor.o: source/processor.cpp
	@g++ $(FLAGS) -c source/processor.cpp -o $(BUILD)/processor.o

$(BUILD)/check_args.o: source/check_args.cpp
	@g++ $(FLAGS) -c source/check_args.cpp -o $(BUILD)/check_args.o

$(BUILD)/main.o: main.cpp
	@g++ $(FLAGS) -c main.cpp -o $(BUILD)/main.o

clean:
	rm $(BUILD)/*.o $(BUILD)/output
