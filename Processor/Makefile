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

output: $(BUILD)/taylor.o $(program_OBJ)
	@g++ $(program_OBJ) $(BUILD)/taylor.o $(FLAGS) -o $(BUILD)/output

$(BUILD)/parser.o: source/parser.cpp
	@g++ $(FLAGS) -c source/parser.cpp -o $(BUILD)/parser.o

$(BUILD)/compare_doubles.o: source/compare_doubles.cpp
	@g++ $(FLAGS) -c source/compare_doubles.cpp -o $(BUILD)/compare_doubles.o

$(BUILD)/print_svg.o: source/print_svg.cpp
	@g++ $(FLAGS) -c source/print_svg.cpp -o $(BUILD)/print_svg.o

$(BUILD)/check_args.o: source/check_args.cpp
	@g++ $(FLAGS) -c source/check_args.cpp -o $(BUILD)/check_args.o

$(BUILD)/dif_operations.o: source/dif_operations.cpp
	@g++ $(FLAGS) -c source/dif_operations.cpp -o $(BUILD)/dif_operations.o

$(BUILD)/diff.o: source/diff.cpp
	@g++ $(FLAGS) -c source/diff.cpp -o $(BUILD)/diff.o

$(BUILD)/taylor.o: taylor.cpp
	@g++ $(FLAGS) -c taylor.cpp -o $(BUILD)/taylor.o

clean:
	rm $(BUILD)/*.o $(BUILD)/output
