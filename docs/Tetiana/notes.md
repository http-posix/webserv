```cpp
	static_assert(std::is_trivially_copyable_v<pollfd>);
	static_assert(sizeof(pollfd) < 16);
```