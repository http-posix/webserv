
1. Early return to avoid nesting
2. Explicitness is better than implicitness
3. DRY - Don't repeat yourself – avoid code duplication
4. SRP - Single Responsibility
5. Use Imperative Mood – write commit messages in the imperative mood, which gives a sense of direction for the change. Start the subject with a verb, such as "Fix," "Add," or "Update."
6. Make a function as a member only if it needs direct access to the representation of a class
7. Avoid void return if this function does something more than just printing. It helps support code when something new is added or tracks unexpected errors, without rewriting a whole bunch of code.
8. If you allocate memory (use smart pointers - can we use them? and do we need them?) or open fd or use another resource that should be cleaned later on - leave comment like 'need to check proper cleaning'
9. Make function description full or just a short comment about it
10. You need to have a system for how you track errors, the same logic to have consistency
11. Const-correctness – if method could be const it should be const
12. strace -f -e trace=close (binary_name) 2>&1 | grep 'close(3)'
	check descriptors. especially double close