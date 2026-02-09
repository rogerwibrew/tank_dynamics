# FastAPI Reference

## What Problem Does FastAPI Solve?

Building a REST API traditionally requires writing a lot of boilerplate code:

```python
# WITHOUT FastAPI - Flask or raw ASGI
from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route("/items/<int:item_id>", methods=["GET"])
def get_item(item_id):
    # Manually parse path parameter
    if not isinstance(item_id, int):
        return {"error": "item_id must be integer"}, 400
    
    # Manually parse query parameters
    q = request.args.get("q")
    if q and not isinstance(q, str):
        return {"error": "q must be string"}, 400
    
    # Manually validate and convert types
    # Manually document the endpoint
    # No automatic request validation
    # No automatic response validation
    
    return {"item_id": item_id, "q": q}

# Need separate OpenAPI/Swagger documentation
# Need to manually handle errors
# Need to write validation code everywhere
```

FastAPI eliminates all this boilerplate by:

1. **Automatic Parameter Validation** - Type hints define and validate request data
2. **Automatic API Documentation** - Interactive Swagger UI at `/docs`, ReDoc at `/redoc`
3. **Type Safety** - Get IDE autocomplete and static type checking
4. **Automatic Request Parsing** - JSON bodies, query params, path params all handled
5. **Automatic Response Serialization** - Return Python objects, FastAPI converts to JSON
6. **Error Handling** - Clear, structured validation errors
7. **Async Support** - First-class async/await support for high concurrency

With FastAPI:

```python
from fastapi import FastAPI

app = FastAPI()

@app.get("/items/{item_id}")
async def get_item(item_id: int, q: str | None = None):
    # Type hints define everything
    # Validation happens automatically
    # Documentation generated automatically
    # IDE knows item_id is int, q is Optional[str]
    return {"item_id": item_id, "q": q}
```

That's it. FastAPI handles validation, documentation, type conversion, and error responses automatically.

---

## Getting Started

### Installation

```bash
pip install fastapi uvicorn
```

- **fastapi**: The web framework
- **uvicorn**: The ASGI server to run the app

### Minimal Application

```python
from fastapi import FastAPI

app = FastAPI()

@app.get("/")
async def read_root():
    return {"message": "Hello, World!"}
```

Run with:
```bash
uvicorn main:app --reload
```

Access:
- API: `http://localhost:8000/`
- Interactive docs (Swagger): `http://localhost:8000/docs`
- Alternative docs (ReDoc): `http://localhost:8000/redoc`

---

## HTTP Methods and Routes

### Basic Decorators

FastAPI uses decorators to define routes:

```python
from fastapi import FastAPI

app = FastAPI()

@app.get("/items")              # GET
async def list_items():
    return [{"id": 1, "name": "Widget"}]

@app.post("/items")             # POST
async def create_item(data: dict):
    return {"created": True}

@app.get("/items/{item_id}")    # GET with path parameter
async def get_item(item_id: int):
    return {"item_id": item_id}

@app.put("/items/{item_id}")    # PUT
async def update_item(item_id: int, data: dict):
    return {"item_id": item_id, "updated": True}

@app.delete("/items/{item_id}") # DELETE
async def delete_item(item_id: int):
    return {"deleted": True}

@app.patch("/items/{item_id}")  # PATCH
async def patch_item(item_id: int, data: dict):
    return {"item_id": item_id, "patched": True}
```

---

## Path Parameters

Path parameters are required URL segments:

```python
@app.get("/items/{item_id}")
async def get_item(item_id: int):
    # item_id is required
    # Must be an integer (automatic validation)
    return {"item_id": item_id}

# GET /items/42 ✓ returns {"item_id": 42}
# GET /items/abc ✗ returns 422 validation error
# GET /items/ ✗ 404 not found (path doesn't match)
```

Multiple path parameters:

```python
@app.get("/users/{user_id}/items/{item_id}")
async def get_user_item(user_id: int, item_id: int):
    return {"user_id": user_id, "item_id": item_id}

# GET /users/1/items/42 ✓
```

Type validation on path parameters:

```python
from enum import Enum

class ItemType(str, Enum):
    WEAPON = "weapon"
    ARMOR = "armor"
    CONSUMABLE = "consumable"

@app.get("/items/{item_type}")
async def get_by_type(item_type: ItemType):
    # Only accepts "weapon", "armor", or "consumable"
    return {"type": item_type}

# GET /items/weapon ✓
# GET /items/unknown ✗ 422 validation error
```

---

## Query Parameters

Query parameters are optional URL query strings (`?key=value`):

```python
@app.get("/items")
async def list_items(skip: int = 0, limit: int = 10):
    # Both optional, with defaults
    return {"skip": skip, "limit": limit}

# GET /items ✓ {"skip": 0, "limit": 10}
# GET /items?skip=5 ✓ {"skip": 5, "limit": 10}
# GET /items?skip=5&limit=20 ✓ {"skip": 5, "limit": 20}
# GET /items?skip=abc ✗ 422 validation error
```

Optional query parameters:

```python
@app.get("/items")
async def list_items(q: str | None = None):
    # q is optional
    if q:
        return {"query": q}
    return {"query": "no query provided"}

# GET /items ✓ {"query": "no query provided"}
# GET /items?q=widget ✓ {"query": "widget"}
```

List query parameters:

```python
@app.get("/items")
async def list_items(tags: list[str] = []):
    return {"tags": tags}

# GET /items?tags=python&tags=api ✓ {"tags": ["python", "api"]}
# GET /items ✓ {"tags": []}
```

Type validation on query parameters:

```python
@app.get("/items")
async def list_items(skip: int = 0, limit: int = 10):
    # FastAPI converts "5" to 5 automatically
    # Returns 422 if conversion fails
    return {"skip": skip, "limit": limit}

# GET /items?skip=5&limit=20 ✓
# GET /items?skip=abc ✗ 422 validation error
```

---

## Request Body

The request body is JSON sent in a POST/PUT request. Use Pydantic models to define the structure:

```python
from pydantic import BaseModel

class Item(BaseModel):
    name: str
    price: float
    description: str | None = None

@app.post("/items")
async def create_item(item: Item):
    # item is automatically validated and parsed
    return item

# POST /items
# {"name": "Widget", "price": 19.99}
# ✓ returns {"name": "Widget", "price": 19.99, "description": null}

# POST /items
# {"name": "Widget"}
# ✗ 422 validation error (price is required)

# POST /items
# {"name": "Widget", "price": "not a number"}
# ✗ 422 validation error (price must be float)
```

The decorated function parameter must match a Pydantic model type. FastAPI will:
- Parse the JSON body
- Validate it against the model
- Convert types (e.g., `"19.99"` → `19.99`)
- Pass the validated model instance to your function
- Return a 422 error if validation fails

Multiple request bodies (use `Body()`):

```python
from fastapi import Body
from pydantic import BaseModel

class Item(BaseModel):
    name: str
    price: float

class Discount(BaseModel):
    percent: float

@app.post("/items")
async def create_item(
    item: Item,
    discount: Discount = Body(...),
):
    # Multiple models in request body
    return {"item": item, "discount": discount}

# POST /items
# {
#     "item": {"name": "Widget", "price": 19.99},
#     "discount": {"percent": 10}
# }
```

---

## Mixing Parameters

A single endpoint can mix path, query, and body parameters:

```python
from pydantic import BaseModel

class Item(BaseModel):
    name: str
    price: float

@app.put("/items/{item_id}")
async def update_item(
    item_id: int,           # Path parameter
    item: Item,             # Request body (Pydantic model)
    q: str | None = None,   # Query parameter
):
    return {
        "item_id": item_id,
        "item": item,
        "query": q
    }

# PUT /items/42?q=search
# {"name": "Updated Widget", "price": 29.99}
# ✓ All parameters validated and passed correctly
```

FastAPI automatically distinguishes:
- **Path parameters**: In the path template (`{item_id}`)
- **Query parameters**: Plain function arguments with defaults
- **Request body**: Pydantic model arguments (not a basic type)

---

## Response Models

Specify what data your endpoint returns with a Pydantic model:

```python
from pydantic import BaseModel

class ItemResponse(BaseModel):
    id: int
    name: str
    price: float

@app.get("/items/{item_id}", response_model=ItemResponse)
async def get_item(item_id: int):
    # Return any dict or object
    # FastAPI validates against ItemResponse
    # Swagger docs show ItemResponse structure
    return {"id": item_id, "name": "Widget", "price": 19.99}
```

Benefits:
- **Documentation**: API consumers see exactly what fields they get back
- **Validation**: FastAPI ensures response matches the model (dev catches bugs early)
- **Filtering**: Can return extra fields in the dict, only the model fields are serialized

Response with nested models:

```python
class Address(BaseModel):
    street: str
    city: str

class User(BaseModel):
    id: int
    name: str
    address: Address

@app.get("/users/{user_id}", response_model=User)
async def get_user(user_id: int):
    return {
        "id": user_id,
        "name": "John",
        "address": {"street": "Main St", "city": "NYC"}
    }
```

---

## Status Codes

Set custom status codes for responses:

```python
from fastapi import FastAPI, status

@app.post("/items", status_code=status.HTTP_201_CREATED)
async def create_item(item: dict):
    return item

# Returns 201 Created instead of 200 OK
```

Common status codes:
- `200` - OK (default)
- `201` - Created
- `204` - No Content
- `400` - Bad Request
- `401` - Unauthorized
- `403` - Forbidden
- `404` - Not Found
- `422` - Unprocessable Entity (validation error)
- `500` - Internal Server Error

---

## Exception Handling

FastAPI provides built-in HTTP exceptions:

```python
from fastapi import HTTPException, status

@app.get("/items/{item_id}")
async def get_item(item_id: int):
    if item_id < 1:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="item_id must be positive"
        )
    
    item = database.get(item_id)
    if not item:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Item {item_id} not found"
        )
    
    return item

# Invalid: GET /items/-1 ✗ 400 {"detail": "item_id must be positive"}
# Not found: GET /items/999 ✗ 404 {"detail": "Item 999 not found"}
```

Validation errors are automatic - FastAPI returns 422 with detailed error info:

```python
from pydantic import BaseModel

class Item(BaseModel):
    name: str
    price: float

@app.post("/items")
async def create_item(item: Item):
    return item

# POST with missing price:
# {
#     "detail": [
#         {
#             "type": "missing",
#             "loc": ["body", "price"],
#             "msg": "Field required"
#         }
#     ]
# }
```

---

## Dependencies

Use dependencies for code that runs before your endpoint. Common uses: auth, database connections, validation:

```python
from fastapi import Depends, HTTPException

async def verify_token(token: str) -> dict:
    """Validate token and return user info"""
    if token != "valid-token":
        raise HTTPException(status_code=401, detail="Invalid token")
    return {"user_id": 1, "username": "john"}

@app.get("/protected")
async def protected_route(user: dict = Depends(verify_token)):
    # verify_token runs first, its result is passed to your function
    return {"message": f"Hello {user['username']}"}

# GET /protected?token=valid-token ✓ {"message": "Hello john"}
# GET /protected?token=invalid ✗ 401 {"detail": "Invalid token"}
```

Dependencies with path parameters:

```python
async def get_item_from_db(item_id: int) -> dict:
    item = database.get(item_id)
    if not item:
        raise HTTPException(status_code=404, detail="Item not found")
    return item

@app.get("/items/{item_id}/details")
async def get_item_details(item: dict = Depends(get_item_from_db)):
    return item
```

---

## Middleware

Middleware runs before/after every request:

```python
from fastapi import FastAPI
from starlette.middleware.cors import CORSMiddleware

app = FastAPI()

# Add CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:3000"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# Add custom middleware
@app.middleware("http")
async def add_process_time_header(request, call_next):
    start_time = time.time()
    response = await call_next(request)
    process_time = time.time() - start_time
    response.headers["X-Process-Time"] = str(process_time)
    return response
```

---

## Startup and Shutdown Events

Run code when the app starts or stops:

```python
from fastapi import FastAPI

app = FastAPI()

@app.on_event("startup")
async def startup():
    print("App starting...")
    # Initialize database connection
    # Load configuration
    # etc.

@app.on_event("shutdown")
async def shutdown():
    print("App shutting down...")
    # Close database connection
    # Cleanup resources
    # etc.
```

---

## Async vs Sync

FastAPI supports both async and sync endpoints:

```python
# Async (preferred for I/O operations)
@app.get("/items")
async def list_items():
    items = await database.query("SELECT * FROM items")
    return items

# Sync
@app.get("/items")
def list_items():
    items = database.query("SELECT * FROM items")  # blocking call
    return items
```

Async is preferred for:
- Database queries
- HTTP requests to other services
- File I/O
- Any operation that can be concurrent

Async allows FastAPI to handle thousands of concurrent requests efficiently.

---

## Automatic Documentation

FastAPI generates interactive API documentation automatically:

```python
from fastapi import FastAPI
from pydantic import BaseModel

class Item(BaseModel):
    """An item in inventory"""
    name: str
    price: float
    description: str | None = None

app = FastAPI(title="My API", description="My awesome API")

@app.post("/items", response_model=Item, status_code=201)
async def create_item(item: Item):
    """Create a new item"""
    return item

@app.get("/items/{item_id}", response_model=Item)
async def get_item(item_id: int):
    """Get an item by ID"""
    return {"id": item_id, "name": "Widget", "price": 19.99}
```

Swagger UI at `/docs` shows:
- All endpoints
- Expected request/response formats
- Status codes
- "Try it out" button to test endpoints
- Docstrings as descriptions

ReDoc at `/redoc` shows:
- Similar info in a different layout
- Better for documentation viewing

---

## Common Patterns

### Pagination

```python
@app.get("/items")
async def list_items(skip: int = 0, limit: int = 10):
    # Typical pagination: skip N items, return next M
    items = database.query(skip=skip, limit=limit)
    return items

# GET /items?skip=0&limit=10
# GET /items?skip=10&limit=10
```

### Filtering

```python
@app.get("/items")
async def list_items(
    skip: int = 0,
    limit: int = 10,
    category: str | None = None,
    min_price: float | None = None,
):
    query = database.query()
    if category:
        query = query.filter(category=category)
    if min_price:
        query = query.filter(price__gte=min_price)
    
    return query.offset(skip).limit(limit).all()

# GET /items?category=weapon&min_price=10.0
```

### Create and Return

```python
from pydantic import BaseModel

class ItemCreate(BaseModel):
    name: str
    price: float

class ItemResponse(BaseModel):
    id: int
    name: str
    price: float

@app.post("/items", response_model=ItemResponse, status_code=201)
async def create_item(item: ItemCreate):
    # Separate input/output models
    db_item = database.create(item.name, item.price)
    return db_item

# POST /items
# {"name": "Widget", "price": 19.99}
# ✓ 201 {"id": 1, "name": "Widget", "price": 19.99}
```

---

## Key Points

- **Type hints are everything** - Define request/response structure with types and Pydantic models
- **Validation is automatic** - FastAPI validates all inputs automatically
- **Documentation is automatic** - Interactive Swagger UI generated from code
- **Async is first-class** - Build highly concurrent APIs naturally
- **Errors are helpful** - 422 validation errors clearly show what's wrong
- **Type safety** - IDE autocomplete for request/response data
- **Minimal boilerplate** - Focus on business logic, not infrastructure

