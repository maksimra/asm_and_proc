call main
hlt

main:
        in
        pop rax
        call factorial
        push rbx
        out
ret

factorial:
        push rax
        push 1
        je end_recursion

        push rax

        push rax
        push 1
        sub
        pop rax

        call factorial

        push rbx
        mul
        pop rbx
ret

        end_recursion:
                push 1
                pop rbx
        ret

