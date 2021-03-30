# 4. How to Deal With Strings

This section explains how strings are represented in Python 2.x, Python 3.x and GTK+ and discusses common errors that arise when working with strings.

# 4.1. Definitions

Conceptually, a string is list of characters such as 'A', 'B', 'C' or 'É'. __Characters__ are abstract representations and their meaning depends on the language and context they are used in. The Unicode standard describes how characters are represented by __code points__. For example the characters above are represented with the code points U+0041, U+0042, U+0043, and U+00C9, respectively. Basically, code points are numbers in range from 0 to 0x10FFFF.

As mentioned earlier, the representation of a string as a list of code points is abstract. In order to convert this abstract representation into a sequence of bytes, the Unicode string must be __encoded__. The simplest form of encoding is ASCII and is performed as follows:

1. If the code point is < 128, each byte is the same as the value of the code point.
2. If the code point is 128 or greater, the Unicode string can't be represented in this encoding. (Python raises a `UnicodeEncodeError` exception in this case.)

Although ASCII encoding is simple to apply, it can only encode for 128 different characters which is hardly enough. One of the most commonly used encodings that addresses this problem is UTF-8 (it can handle any Unicode code point). UTF stands for "Unicode Transformation Format", and the '8' means that 8-bit numbers are used in the encoding.

# 4.2. Python 2

# 4.2.1. Python 2.x's Unicode Support

Python 2 comes with two different kinds of objects that can be used to represent strings, `str` and `unicode`. Instances of the latter are used to express Unicode strings, whereas instances of the `str` type are byte representations (the encoded string). Under the hood, Python represents Unicode strings as either 16- or 32-bit integers, depending on how the Python interpreter was compiled. Unicode strings can be converted to 8-bit strings with `unicode.encode()`:

```py
unicode_string = u'Fu\u00dfb\u00e4lle'
print unicode_string # Fußbälle
type(unicode_string) # <type 'unicode'>
unicode_string.encode('utf-8') # 'Fu\xc3\x9fb\xc3\xa4lle'
```

Python's 8-bit strings have a `str.decode()` method that interprets the string using the given encoding:

```py
utf8_string = unicode_string.encode('utf-8')
type(utf8_string) # <type 'str'>
u2 = utf8_string.decode('utf-8')
unicode_string == u2 # True
```

Unfortunately, Python 2.x allows you to mix `unicode` and `str` if the 8-bit string happended to contain only 7-bit (ASCII) bytes, but would get `UnicodeDecodeError` if it contained non-ASCII values:

<!-- TODO -->

# 4.2.2. Unicode in GTK+

GTK+ uses UTF-8 encoded strings for all text. This means that if you call a method that returns a string you will always obtain an instance of the `str` type. The same applies to methods that expect one or more strings as parameter, they must be UTF-8 encoded. However, for convenience PyGObject will automatically convert any `unicode` instance to `str` if supplied as argument:

<!-- TODO -->

# 4.3. Python 3

# 4.3.1. Python 3.x's Unicode support

Since Python 3.0, all strings are stored as Unicode in an instance of the `str` type. `Encoded` strings on the other hand are represented as binary data in the form of instances of `bytes` type. Conceptually, `str` refers to _text_, whereas `bytes` refers to _data_. Use `str.encode()` to go from `str` to `bytes`, and `bytes.decode()` to go from `bytes` to `str`.

In addition, it is no longer possible to mix Unicode strings with encoded strings, because it will result in a `TypeError`:

<!-- TODO -->

# 4.3.2. Unicode in GTK+

As a consequence, things are much cleaner and consistent with Python 3.x, because PyGObject will automatically encode/decode to/from UTF-8 if you pass a string to a method or a method returns a string. Strings, or _text_ will always be represented as instances of `str` only:

<!-- TODO -->
