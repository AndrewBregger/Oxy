# Syntax
Pratt is an expression based language such as Rust.
## Keywords
- if
- else
- let
- mut
- fn
- type
- break
- continue
- return
- while
- for
- match
- type
- sizeof
- alignof
- when
- use

## Declarations
### Variable Declarations
By default, variables are declared as constant.
```
let x = 1.0f32
```

The mut keyword is used to allow a variable to be mutated (Rust syntax). ':=' syntax can also be used for mutable variable declaration. 
```
let mut y = 3.32f64
let mut z = Vector {x: 1.0, y: 3.0}
k := Quat {1.0, 1.0, 1.0, 1.0}
```

### Structure
```
struct Vector {
  x, y, z: f32 = 0
}

struct Vector($T, N: u32) {
  data: [N]T;
}

// this is just a special type alias.
type Error(i32)
```

### Type Alias

```
type MyInt = Int
```

### Function
```
fn new_vector(x, y, z: f32) z: Vector {
  z = Vector {
    x: x,
    y: y,
    z: z
  }
}

let vec = Vector{1, 1}

// the object gets consumed (maybe there should be serperate syntax for this instead of a simple copy)
fn norm(vec: Vector) u32 {
  sqrt(vec.x ** 2 + vec.y ** 2 + vec.z ** 2)
}

// this could be used as the consume notation
fn norm(vec: $Vector) u32 {
  sqrt(vec.x ** 2 + vec.y ** 2 + vec.z ** 2) 
}

// the object is referenced
fn norm(vec: &Vector) u32 {
  sqrt(vec.x ** 2 + vec.y ** 2 + vec.z ** 2)
}

// the object is a pointer
fn norm(vec: *Vector) u32 {
  sqrt(vec.x ** 2 + vec.y ** 2 + vec.z ** 2)
}

norm(vec)

// the operation will be determined by the type checker
norm(&vec) // will get reference
norm(&vec) // will get pointer

// experimental syntax
vec.norm()
```
### Enum
Similar to Rusts enums (kind of makes unions useless)
```
enum Foos {
  Bars,
  Fees(i32, f32)
  Foo(Vector)
}
```

### Type Alias
```
type MyInt = i32

// lets say we have polymoric type
struct Vector(T, N: int) {
  data: [N]T
}

type Vector3f = Vector(f32, 3)
```
## Control Flow
### If
```
if true {
  //...
}
else {
  //...
}

let y = if z {
  1.0f32
}
else {
  2.0f32
}

if x {
  | 1..3 {
    // do something
  },
  | 3..5 {
    // do something else
  },
}
else {
  // default
}

```

### If Let
This might be replaced by a more general if expression.
```
struct Foo(f32, i32);

bar := Foo(1, 2)
if let Foo(x, y) = bar {
  // x = 1
  // y = 2
}
```

### While
```
while x != y {

}
```

### For
```
```

### Match
```
match x {
  1: {

  },
  2: {

  },
  3: {

  },
}
```

# Extendend Bakus Naur Form
