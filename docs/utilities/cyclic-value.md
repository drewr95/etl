---
title: "cyclic_value"
---

{{< callout type="info">}}
  Header: `cyclic_value.h`  
  Since: `TBC`  
{{< /callout >}}

Provides a value that cycles between two limits.  
Supports increments, decrements and arbitrary advance.  
Supports compile time and runtime variants.  

```cpp
template <typename T, const T First = 0, const T Last = 0>
class cyclic_value
```

```cpp
etl::cyclic_value<int, 2, 7> value_ct; // Compile time. Fixed range of 2 to 7 inclusive.

etl::cyclic_value<int> value_rt(2, 7); // Run time. Modifiable range.
value_rt.set(3, 8);
```

## Constructor
```cpp
cyclic_value<int> value;
```
Creates a runtime cyclic value of type int with default constructed initial defined limits.

---

```cpp
cyclic_value<int, N, M> value;
```
Creates a compile time cyclic value of type int with fixed limits of N and M.

**Example**  
```cpp
cyclic_value<int> value(N, M);
```
Creates a runtime cyclic value of type int with initial defined limits of N and M.

As with `clamped_value`, the template arguments `First = 0` and `Last = 0`
select the runtime-bound specialization.

## Modifiers
```cpp
cyclic_value& operator ++() & noexcept;
cyclic_value operator ++(int) noexcept;
```
Increments the value. If the value is at the last value then is set to the first.

---

```cpp
cyclic_value& operator --() & noexcept;
cyclic_value operator --(int) noexcept;
```
Decrements the value. If the value is at the first value then is set to the last.

---

```cpp
void advance(int n) noexcept;
```
Advances the value by the specified amount, wrapping if necessary.

## Access
```cpp
T get() const noexcept;
```
Gets the current value

---

```cpp
static T min() noexcept;
```
Gets the minimum value.

---

```cpp
T max() const noexcept;
```
Gets the maximum value.

---

```cpp
void set(T min, T max) noexcept;
```
Sets the new minimum and maximum values. Sets the current value to min.

---

```cpp
void set(T value) noexcept;
```
Sets the current value.

---

```cpp
void to_min() noexcept;
```
Sets the current value to the minimum value.

---

```cpp
void to_max() noexcept;
```
Sets the current value to the last value

## Operations

```cpp
void swap(cyclic_value<T, FIRST, LAST>& other) noexcept;
```
Swaps with another cyclic value.

---

```cpp
void swap(cyclic_value<T, FIRST, LAST>& lhs, cyclic_value<T, FIRST, LAST>& rhs) noexcept;
```
Swaps with another cyclic value.

## Operators
```cpp
operator T() noexcept;
operator const T() const noexcept;
```
Conversion operators to `T`.

---

```cpp
cyclic_value& operator =(T t) & noexcept;
```
Sets the current value to `t`.

## Non-member functions
```cpp
template <typename T, const T FIRST, const T LAST>
void swap(cyclic_value<T, FIRST, LAST>& lhs, cyclic_value<T, FIRST, LAST>& rhs)
```
Swaps two cyclic values.

---

```cpp
template <typename T, const T FIRST, const T LAST>
bool operator == (cyclic_value<T, FIRST, LAST>& lhs, cyclic_value<T, FIRST, LAST>& rhs)
```
Checks equality of two cyclic values.

---

```cpp
template <typename T, const T FIRST, const T LAST>
bool operator != (cyclic_value<T, FIRST, LAST>& lhs, cyclic_value<T, FIRST, LAST>& rhs)
```
Checks inequality of two cyclic values.
