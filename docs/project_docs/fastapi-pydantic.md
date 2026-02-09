# FastAPI and Pydantic Documentation

## FastAPI Overview

**FastAPI** is a modern, high-performance web framework for building APIs with Python. It combines exceptional speed (on par with NodeJS and Go) with an excellent developer experience, all based on standard Python type hints.

### Key Features

- Built on Starlette for high performance
- Automatic OpenAPI and Swagger documentation
- Type hint-based validation and serialization
- Support for both synchronous and asynchronous endpoints
- Built-in data validation through Pydantic integration
- Easy to learn and deploy

### Getting Started

To create a FastAPI application, import the `FastAPI` class and create an instance:

```python
from fastapi import FastAPI

app = FastAPI()

@app.get("/")
async def read_root():
    return {"Hello": "World"}

@app.get("/items/{item_id}")
async def read_item(item_id: int, q: str | None = None):
    return {"item_id": item_id, "q": q}
```

Run the application with:
```bash
uvicorn main:app --reload
```

Access the interactive API documentation at `http://localhost:8000/docs`.

---

## Pydantic Overview

**Pydantic** is a fast and extensible data validation library for Python that uses type hints to define data schemas. It provides runtime type checking, automatic data conversion, and user-friendly error messages.

### Key Features

- Type hint-based validation
- Automatic data type coercion (e.g., string "123" → integer 123)
- Rich, detailed error reporting
- Supports complex nested models
- Configuration options for strict or flexible validation
- Serialization and JSON schema generation

### Basic Example

Define models by inheriting from `BaseModel`:

```python
from pydantic import BaseModel
from typing import Optional
from datetime import datetime

class User(BaseModel):
    id: int
    name: str = 'John Doe'
    signup_ts: Optional[datetime] = None
    friends: list[int] = []

# Create and validate with external data
external_data = {
    'id': '123',
    'signup_ts': '2017-06-01 12:22',
    'friends': [1, '2', b'3']
}

user = User(**external_data)
print(user.id)  # 123 (converted from string)
print(user)     # User id=123 name='John Doe' signup_ts=datetime(...) friends=[1, 2, 3]
```

Pydantic automatically:
- Converts the string `'123'` to integer
- Parses the datetime string into a `datetime` object
- Coerces the byte string `b'3'` to string `'3'`

### Model Configuration

```python
from pydantic import BaseModel, ConfigDict

class StrictModel(BaseModel):
    model_config = ConfigDict(
        extra='forbid',  # Reject unexpected fields
        title="My Model"
    )
    
    name: str
    age: int
```

---

## FastAPI + Pydantic Integration

FastAPI leverages Pydantic for automatic request and response validation. When you define a parameter as a Pydantic model, FastAPI uses it to:

1. Parse the request body as JSON
2. Validate the data against the model
3. Convert data types as needed
4. Generate automatic API documentation
5. Return detailed validation errors if data is invalid

### Example

```python
from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI()

class Item(BaseModel):
    name: str
    price: float
    description: str | None = None

@app.post("/items/")
async def create_item(item: Item):
    return item
```

When a POST request arrives with JSON:
```json
{
    "name": "Widget",
    "price": "19.99",
    "description": "A useful widget"
}
```

Pydantic automatically:
- Converts `"19.99"` (string) to `19.99` (float)
- Validates all required fields are present
- Generates a 422 validation error if data is invalid

Invalid requests receive detailed error responses, making debugging easier for API consumers.

---

## Summary

- **FastAPI**: High-performance API framework with automatic documentation
- **Pydantic**: Data validation library providing type safety and automatic coercion
- Together: FastAPI uses Pydantic to validate requests/responses with minimal boilerplate code
