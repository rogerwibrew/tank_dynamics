# Pydantic Reference

## What Problem Does Pydantic Solve?

Without Pydantic, you have to write validation code manually:

```python
# WITHOUT Pydantic - you write this:
class User:
    def __init__(self, name, age, email):
        if not isinstance(name, str):
            raise TypeError("name must be str")
        if not isinstance(age, int) or age < 0 or age > 150:
            raise ValueError("age must be int between 0 and 150")
        if not isinstance(email, str):
            raise ValueError("email must be str")
        
        self.name = name
        self.age = age
        self.email = email
```

This is repetitive, error-prone, and hard to maintain. With Pydantic, you declare your schema once using type hints, and Pydantic handles all validation automatically:

```python
# WITH Pydantic - you write this:
from pydantic import BaseModel, Field

class User(BaseModel):
    name: str
    age: int = Field(ge=0, le=150)
    email: str
```

### Key Problems Pydantic Solves

1. **Data Validation** - Automatic type checking and constraint validation (no manual if/raise boilerplate)
2. **Data Serialization** - Convert Python objects to JSON and back, handling edge cases automatically
3. **Data Parsing** - Convert raw input (JSON, dicts) into validated Python objects with proper types
4. **Schema Documentation** - Type hints make the expected data structure explicit and self-documenting
5. **Error Messages** - Clear, structured validation errors instead of vague exceptions

### Real-World Example: API Request Handling

Without Pydantic:
```python
@app.post("/users")
def create_user(request_data):
    # Manually validate everything
    data = request_data.json()
    
    if "name" not in data:
        return {"error": "name is required"}, 400
    if not isinstance(data["name"], str):
        return {"error": "name must be string"}, 400
    if len(data["name"]) == 0:
        return {"error": "name cannot be empty"}, 400
    
    if "age" not in data:
        return {"error": "age is required"}, 400
    if not isinstance(data["age"], int):
        return {"error": "age must be int"}, 400
    if data["age"] < 0 or data["age"] > 150:
        return {"error": "age must be 0-150"}, 400
    
    # ... more validation ...
    
    user = User(data["name"], data["age"], data.get("email"))
    db.save(user)
    return {"success": True}
```

With Pydantic:
```python
from pydantic import BaseModel, Field, ValidationError

class UserCreate(BaseModel):
    name: str = Field(min_length=1)
    age: int = Field(ge=0, le=150)
    email: str = ""

@app.post("/users")
def create_user(user: UserCreate):  # Pydantic validates automatically
    db.save(user)
    return {"success": True}
```

Pydantic parses JSON, validates all fields, returns 400 with detailed errors if validation fails. Zero manual validation code.

---

## Basic Model Definition

Pydantic models are classes that inherit from `BaseModel`. Each field is defined with a type hint:

```python
from pydantic import BaseModel

class User(BaseModel):
    name: str
    age: int
    email: str = "default@example.com"  # optional with default value
```

- Fields without defaults are **required**
- Fields with defaults are **optional** (can be omitted)
- Type hints are enforced - passing wrong types raises `ValidationError`

## Creating Instances

```python
# Valid: all required fields provided
user = User(name="John", age=30, email="john@example.com")

# Valid: uses default for email
user = User(name="Jane", age=25)

# Invalid: missing required field (raises ValidationError)
try:
    user = User(name="Bob")  # age is required
except ValidationError as e:
    print(e)
    # ValidationError: 1 validation error for User
    # age: Field required [type=missing, input_value={...}]

# Invalid: wrong type (raises ValidationError)
try:
    user = User(name="Alice", age="thirty")
except ValidationError as e:
    print(e)
    # ValidationError: 1 validation error for User
    # age: Input should be a valid integer [type=int_parsing, input_value='thirty']
```

## Field Constraints

Use `Field()` to add validation constraints beyond just the type:

```python
from pydantic import BaseModel, Field

class Product(BaseModel):
    name: str = Field(min_length=1, max_length=100, description="Product name")
    price: float = Field(gt=0, description="Must be positive")  # greater than 0
    quantity: int = Field(ge=0, description="Cannot be negative")  # greater than or equal to 0
    sku: str = Field(pattern=r"^[A-Z]{3}-\d{4}$")  # matches regex pattern
```

Common field constraints:
- `gt`, `lt`, `ge`, `le` - numeric comparisons (greater/less than, equal)
- `min_length`, `max_length` - string/list length constraints
- `pattern` - regex validation for strings
- `description` - field documentation (helps with API docs)

Example with violations:
```python
product = Product(name="", price=0, quantity=-1)
# ValidationError: 3 validation errors for Product
# name: String should have at least 1 character [type=string_too_short]
# price: Input should be greater than 0 [type=greater_than]
# quantity: Input should be greater than or equal to 0 [type=greater_than_equal]
```

## Field Validators

For validation logic that can't be expressed with `Field()` constraints, use `@field_validator`. This lets you write custom Python code to validate fields:

```python
from pydantic import BaseModel, field_validator, ValidationError

class User(BaseModel):
    username: str
    age: int
    
    @field_validator('username')
    @classmethod
    def validate_username(cls, v):
        """Custom validation: username must be alphanumeric and 3+ chars"""
        if len(v) < 3:
            raise ValueError('must be at least 3 characters')
        if not v.isalnum():
            raise ValueError('must be alphanumeric only')
        return v
    
    @field_validator('age')
    @classmethod
    def validate_age(cls, v):
        """Custom validation: age must be reasonable"""
        if v < 0 or v > 150:
            raise ValueError('age must be between 0 and 150')
        return v

# Valid
user = User(username="john_doe", age=30)

# Invalid: username too short
try:
    user = User(username="jo", age=30)
except ValidationError as e:
    print(e)  # must be at least 3 characters

# Invalid: username has underscore
try:
    user = User(username="john_doe", age=30)
except ValidationError as e:
    print(e)  # must be alphanumeric only
```

Validators run **after** type validation, so `v` is already the correct type. They can modify and return the value:

```python
class Product(BaseModel):
    name: str
    
    @field_validator('name')
    @classmethod
    def normalize_name(cls, v):
        return v.strip().title()  # normalize: trim and capitalize

product = Product(name="  laptop computer  ")
print(product.name)  # "Laptop Computer"
```

## Error Handling

Pydantic raises `ValidationError` when data doesn't match the schema. Catch and inspect it:

```python
from pydantic import ValidationError

try:
    user = User(name="John", age="not_a_number")
except ValidationError as e:
    # Human-readable summary
    print(e)
    # 1 validation error for User
    # age: Input should be a valid integer [type=int_parsing]
    
    # Structured error details (useful for API responses)
    print(e.errors())
    # [
    #     {
    #         'type': 'int_parsing',
    #         'loc': ('age',),
    #         'msg': 'Input should be a valid integer',
    #         'input': 'not_a_number'
    #     }
    # ]
```

In a web API, use the structured errors to return helpful JSON responses:

```python
from fastapi import FastAPI
from pydantic import ValidationError

@app.post("/users")
def create_user(data: dict):
    try:
        user = User(**data)
        return user
    except ValidationError as e:
        return {"errors": e.errors()}, 400
```

## Nested Models

Models can contain other models. Pydantic validates the entire nested structure:

```python
from pydantic import BaseModel

class Address(BaseModel):
    street: str
    city: str
    country: str

class Person(BaseModel):
    name: str
    address: Address

# Create with nested dict - Pydantic converts automatically
person = Person(
    name="John",
    address={"street": "Main St", "city": "NYC", "country": "USA"}
)

# Or create with nested object
address = Address(street="Main St", city="NYC", country="USA")
person = Person(name="John", address=address)

# Validation happens on nested data too
try:
    person = Person(
        name="John",
        address={"street": "Main St", "city": "NYC"}  # missing country
    )
except ValidationError as e:
    print(e)  # country: Field required
```

## List and Dict Types

Collections can have typed elements:

```python
from pydantic import BaseModel, Field

class Team(BaseModel):
    name: str
    members: list[str]  # list of strings only
    scores: dict[str, int]  # dict mapping string keys to int values
    tags: set[str]  # set of strings

# Valid
team = Team(
    name="Team A",
    members=["Alice", "Bob", "Charlie"],
    scores={"Alice": 100, "Bob": 95},
    tags={"python", "backend"}
)

# Invalid: members contains non-string
try:
    team = Team(name="Team A", members=["Alice", 123])
except ValidationError as e:
    print(e)  # members.1: Input should be a valid string

# Can also validate nested models
class Company(BaseModel):
    name: str
    teams: list[Team]  # list of Team objects

company = Company(
    name="Acme",
    teams=[
        {"name": "Team A", "members": ["Alice"], "scores": {}, "tags": []},
        {"name": "Team B", "members": ["Bob"], "scores": {}, "tags": []},
    ]
)
```

## Model Validation Methods

Methods for creating and exporting models:

```python
# Create from dict (same as constructor, but explicit)
user = User.model_validate({"name": "John", "age": 30})

# Create from dict, skip validation (faster, use carefully)
user = User.model_validate({"name": "John", "age": 30}, from_attributes=True)

# Serialize to dict
user_dict = user.model_dump()
# {'name': 'John', 'age': 30, 'email': 'default@example.com'}

# Serialize to JSON string
user_json = user.model_dump_json()
# '{"name":"John","age":30,"email":"default@example.com"}'

# Serialize only some fields
partial = user.model_dump(include={'name', 'age'})
# {'name': 'John', 'age': 30}

# Exclude some fields
partial = user.model_dump(exclude={'email'})
# {'name': 'John', 'age': 30}
```

## Configuration

Control model behavior with `model_config`:

```python
from pydantic import ConfigDict, BaseModel

class User(BaseModel):
    model_config = ConfigDict(
        validate_assignment=True,  # validate when you assign to fields
        use_enum_values=True,      # use enum values in serialization, not enum objects
        strict=False,              # allow type coercion (e.g., "30" -> 30)
    )
    name: str
    age: int

user = User(name="John", age=30)

# With validate_assignment=True, this validates immediately
user.age = "not a number"  # raises ValidationError
```

## Type Support

Pydantic supports a wide range of types:

- **Basic types**: `str`, `int`, `float`, `bool`, `bytes`
- **Collections**: `list`, `dict`, `set`, `tuple`
- **Special types**: `datetime`, `UUID`, `URL`, `EmailStr`
- **Enums**: `Enum` for restricted choices
- **Custom classes**: Your own classes (if they're Pydantic models)
- **Optional and Union**: `Optional[T]` or `T | None` for optional fields, `Union[Type1, Type2]` for multiple types

Example with special types:

```python
from pydantic import BaseModel, EmailStr
from datetime import datetime
from uuid import UUID

class Event(BaseModel):
    id: UUID
    name: str
    email: EmailStr
    timestamp: datetime
    category: str  # could be enum instead

# Pydantic parses and validates special types
event = Event(
    id="12345678-1234-5678-1234-567812345678",  # string parsed to UUID
    name="Conference",
    email="user@example.com",
    timestamp="2024-02-08T10:30:00",  # string parsed to datetime
    category="work"
)
```

## Common Patterns

### Optional Fields

```python
from pydantic import BaseModel
from typing import Optional

class User(BaseModel):
    name: str
    nickname: Optional[str] = None  # can be None
    bio: str | None = None  # modern Python syntax

user = User(name="John")  # nickname and bio default to None
user = User(name="John", nickname="JD")  # nickname set
```

### Default Factories

For mutable defaults (lists, dicts), use `Field(default_factory=...)`:

```python
from pydantic import BaseModel, Field
from typing import List

class Team(BaseModel):
    name: str
    members: List[str] = Field(default_factory=list)  # each team gets its own list

team1 = Team(name="Team A")
team2 = Team(name="Team B")
team1.members.append("Alice")

print(team2.members)  # [] (not ['Alice'])
```

### Enums for Restricted Values

```python
from enum import Enum
from pydantic import BaseModel

class Status(str, Enum):
    PENDING = "pending"
    ACTIVE = "active"
    INACTIVE = "inactive"

class User(BaseModel):
    name: str
    status: Status

user = User(name="John", status="active")  # string converted to enum
print(user.status)  # Status.ACTIVE

# Invalid value raises ValidationError
try:
    user = User(name="John", status="unknown")
except ValidationError:
    print("Invalid status")
```

## Key Points

- **Type hints are enforced** - passing wrong types raises `ValidationError`
- **Validation happens on instantiation** - catch errors immediately
- **Models are immutable by default** - reassigning fields raises error (use `ConfigDict(validate_assignment=True)` to allow)
- **Supports nested models** - validate complex data structures automatically
- **Clear error messages** - helps with debugging and API error responses
- **Optional fields** - use `Optional[Type]` or `Type | None`, provide defaults
- **Perfect for APIs** - automatically validate incoming requests and serialize responses

