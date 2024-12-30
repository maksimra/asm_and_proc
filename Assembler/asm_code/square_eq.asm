call main
hlt

main:
        call get_coeff
        call solver
        call print_solutions
        ret

get_coeff:
        in
        pop rax
        in
        pop rbx
        in
        pop rcx
        ret

print_solutions:
        push rax
        push 0
        je zero_solutions

        push rax
        push 3
        je inf_solutions

        push rax
        push 1
        je one_solution

        push rax
        push 2
        je two_solutions

        zero_solutions:
                push 0
                out
                ret

        inf_solutions:
                push -1
                out
                ret

        one_solution:
                push 1
                out
                push rbx
                out
                ret

        two_solutions:
                push 2
                out
                push rbx
                out
                push rcx
                out
                ret

solver:
        push rax
        push 0
        je solve_linear_eq

        push rbx
        push rbx
        mul

        push -4
        push rax
        push rcx
        mul
        mul

        add
        pop rdx

        push rdx
        push 0
        ja negative_discr

        push rdx
        push 0
        je zero_discr

        push rbx
        push -1
        mul

        push rdx
        sqrt
        push -1
        mul

        add

        push rax
        push 2
        mul

        div

        pop rcx

        push rbx
        push -1
        mul

        push rdx
        sqrt

        add

        push rax
        push 2
        mul

        div

        pop rbx

        push 2
        pop rax
        ret

        negative_discr:
                push 0
                pop rax
                ret

        zero_discr:
                push rbx
                push -1
                mul

                push rax
                push 2
                mul
                div

                push 1
                pop rax
                pop rbx
                ret

solve_linear_eq:
        push rbx
        push 0
        je is_zero_b

        jmp b_not_zero

        is_zero_b:
                push rcx
                push 0
                je is_zero_b_and_c
                jmp c_not_zero

                is_zero_b_and_c:
                        push 3
                        pop rax
                        ret

                c_not_zero:
                        push 0
                        pop rax
                        ret

        b_not_zero:
                push rcx
                push -1
                mul

                push rbx
                div

                push 1
                pop rax
                pop rbx
                ret
