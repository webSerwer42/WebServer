# 🎨 Wizualizacja Systemu Obsługi Błędów

## 📊 Przepływ Błędów w Serwerze

```
┌─────────────────────────────────────────────────────────────────┐
│                    KLIENT WYSYŁA ŻĄDANIE                        │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
                ┌────────────────────────┐
                │   recv() w poll.cpp    │
                └────────┬───────────────┘
                         │
         ┌───────────────┼───────────────┐
         │               │               │
         ▼               ▼               ▼
    byteRecived    byteRecived       byteRecived
       == -1           == 0            > 0
         │               │               │
         ▼               ▼               │
    🚨 500          📪 Close            │
   recv failed     connection           │
                                        ▼
                           ┌────────────────────────┐
                           │   WALIDACJA ŻĄDANIA    │
                           └────────┬───────────────┘
                                    │
                    ┌───────────────┼───────────────┐
                    │               │               │
                    ▼               ▼               ▼
               requestStr       request.find    buffer full
                 .empty()       ("HTTP") fail   (size-1)
                    │               │               │
                    ▼               ▼               ▼
               🚨 400          🚨 400          🚨 413
            Empty request   Malformed       Payload
                           HTTP            Too Large
                                │
                                ▼
                   ┌────────────────────────┐
                   │   PRZETWARZANIE        │
                   │   sendToClient()       │
                   └────────┬───────────────┘
                            │
            ┌───────────────┼───────────────┐
            │               │               │
            ▼               ▼               ▼
       try { ... }     send() fails    Exception
       SUCCESS                              │
            │               │               │
            ▼               ▼               ▼
        ✅ 200          🚨 500          🚨 500
        OK            Send failed     Internal
                                      Error
```

---

## 🔄 Życie Żądania HTTP

### 1️⃣ Klient Wysyła Żądanie

```
┌─────────┐
│ KLIENT  │  GET /index.html HTTP/1.1
└────┬────┘  Host: localhost:8080
     │
     │ (socket connection)
     │
     ▼
┌─────────┐
│ SERWER  │  accept() → clientFD
└─────────┘
```

### 2️⃣ Odbieranie Danych (recivNClose)

```
┌───────────────────────────────────────────┐
│         char buffer[8192]                 │
│         memset(buffer, 0, ...)            │
└───────────────┬───────────────────────────┘
                │
                ▼
┌───────────────────────────────────────────┐
│  byteRecived = recv(fd, buffer, 8191, 0)  │
└───────────────┬───────────────────────────┘
                │
                ▼
        ┌───────────────┐
        │ SPRAWDZENIA:  │
        ├───────────────┤
        │ 1. recv == -1?│ → 500 Internal Server Error
        │ 2. empty()?   │ → 400 Bad Request
        │ 3. "HTTP"?    │ → 400 Bad Request
        │ 4. buffer full│ → 413 Content Too Large
        └───────────────┘
```

### 3️⃣ Parsowanie HTTP (Http.cpp)

```
┌───────────────────────────────────────────┐
│  Http response(requestStr, serversCfg)    │
└───────────────┬───────────────────────────┘
                │
                ▼
        ┌───────────────┐
        │ SPRAWDZENIA:  │
        ├───────────────┤
        │ 1. Method?    │ → 405 Method Not Allowed
        │ 2. URI len?   │ → 414 URI Too Long
        │ 3. Version?   │ → 505 HTTP Version Not Supported
        │ 4. Headers?   │ → 431 Request Header Too Large
        └───────────────┘
```

### 4️⃣ Routing (router.cpp)

```
┌───────────────────────────────────────────┐
│  Router::findRoute(path, method)          │
└───────────────┬───────────────────────────┘
                │
                ▼
        ┌───────────────┐
        │ SPRAWDZENIA:  │
        ├───────────────┤
        │ 1. Route?     │ → 404 Not Found
        │ 2. Method OK? │ → 405 Method Not Allowed
        │ 3. Auth?      │ → 401 Unauthorized
        │ 4. Perms?     │ → 403 Forbidden
        └───────────────┘
```

### 5️⃣ CGI Execution (cgi.cpp)

```
┌───────────────────────────────────────────┐
│  executeCgi(scriptPath, interpreter)      │
└───────────────┬───────────────────────────┘
                │
                ▼
        ┌───────────────┐
        │ SPRAWDZENIA:  │
        ├───────────────┤
        │ 1. File exist?│ → 404 Not Found
        │ 2. Executable?│ → 403 Forbidden
        │ 3. execve()?  │ → 502 Bad Gateway
        │ 4. Timeout?   │ → 504 Gateway Timeout
        └───────────────┘
```

### 6️⃣ Wysyłanie Odpowiedzi

```
┌───────────────────────────────────────────┐
│  std::string responseStr =                │
│    response.responseBuilder()             │
└───────────────┬───────────────────────────┘
                │
                ▼
┌───────────────────────────────────────────┐
│  HttpError::generateErrorResponse(code)   │
└───────────────┬───────────────────────────┘
                │
                ▼
┌───────────────────────────────────────────┐
│  1️⃣ hasCustomErrorPage(code)?            │
└───────────────┬───────────────────────────┘
                │
        ┌───────┴────────┐
        │                │
        ▼                ▼
      TRUE             FALSE
        │                │
        ▼                │
┌──────────────────┐     │
│ 2️⃣ fileExists() │     │
│  & isValidPath() │     │
└────────┬─────────┘     │
         │               │
    ┌────┴─────┐         │
    │          │         │
    ▼          ▼         │
   OK        FAIL        │
    │          │         │
    ▼          │         │
┌──────────┐   │         │
│3️⃣ Read   │   │         │
│  File    │   │         │
└────┬─────┘   │         │
     │         │         │
 ┌───┴───┐     │         │
 │       │     │         │
 ▼       ▼     │         │
OK    EMPTY    │         │
 │       │     │         │
 │       └─────┼─────────┤
 │             │         │
 ▼             ▼         ▼
┌─────────┐ ┌─────────────┐
│ CUSTOM  │ │  DEFAULT    │
│  PAGE   │ │   PAGE      │
│ (HTML)  │ │ (gradient)  │
└────┬────┘ └──────┬──────┘
     │             │
     └──────┬──────┘
            │
            ▼
┌───────────────────────────────────────────┐
│  send(clientFD, responseStr.c_str(), ...) │
└───────────────┬───────────────────────────┘
                │
        ┌───────┴───────┐
        │               │
        ▼               ▼
    SUCCESS         FAILED
        │               │
        ▼               ▼
    ✅ 200          🚨 500
```

---

## 🎨 System Niestandardowych Stron Błędów

### 📋 Przepływ Decyzyjny

```
┌─────────────────────────────────────────────┐
│     BŁĄD WYKRYTY (np. 404, 500, 403)       │
└────────────────┬────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────┐
│  httpError.generateErrorResponse(code)      │
└────────────────┬────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────┐
│  bool hasCustom =                           │
│    httpError.hasCustomErrorPage(code)       │
└────────────────┬────────────────────────────┘
                 │
         ┌───────┴────────┐
         │                │
         ▼                ▼
      hasCustom       !hasCustom
       = true          = false
         │                │
         ▼                │
┌──────────────────┐      │
│ Pobierz ścieżkę: │      │
│ getCustomError   │      │
│ PagePath(code)   │      │
└────────┬─────────┘      │
         │                │
         ▼                │
┌──────────────────┐      │
│ fileExists(path)?│      │
└────────┬─────────┘      │
         │                │
    ┌────┴─────┐          │
    │          │          │
    ▼          ▼          │
   YES        NO          │
    │          │          │
    ▼          │          │
┌──────────┐   │          │
│isValidPath│  │          │
│  (path)?  │  │          │
└─────┬─────┘  │          │
      │        │          │
 ┌────┴────┐   │          │
 │         │   │          │
 ▼         ▼   │          │
YES       NO   │          │
 │         │   │          │
 ▼         │   │          │
┌─────────┐│   │          │
│readFile ││   │          │
│Content()││   │          │
└────┬────┘│   │          │
     │     │   │          │
 ┌───┴───┐ │   │          │
 │       │ │   │          │
 ▼       ▼ │   │          │
OK    EMPTY│   │          │
 │       │ │   │          │
 │       └─┼───┼──────────┤
 │         │   │          │
 ▼         ▼   ▼          ▼
┌─────────┐ ┌──────────────┐
│ RETURN  │ │   FALLBACK:  │
│ CUSTOM  │ │ generateHtml │
│  HTML   │ │  Body() with │
│ CONTENT │ │   defaults   │
└─────────┘ └──────────────┘
```

### 🔧 Przykład użycia w kodzie

```cpp
// Inicjalizacja HttpError
HttpError httpError;

// Ustaw niestandardowe strony błędów
httpError.setCustomErrorPage(404, "/var/www/errors/404.html");
httpError.setCustomErrorPage(500, "/var/www/errors/500.html");
httpError.setCustomErrorPage(403, "/var/www/errors/forbidden.html");

// Generowanie odpowiedzi błędu
std::string response = httpError.generateErrorResponse(404);
// Automatycznie sprawdzi czy istnieje /var/www/errors/404.html
// Jeśli TAK i jest dostępny → użyje custom page
// Jeśli NIE → użyje domyślnej strony z gradientem

send(clientFD, response.c_str(), response.length(), 0);
```

### 📂 Struktura plików

```
/var/www/
├── html/
│   ├── index.html
│   └── about.html
└── errors/
    ├── 404.html          ← Custom 404 (musi istnieć!)
    ├── 500.html          ← Custom 500 (musi istnieć!)
    ├── 403.html          ← Custom 403 (musi istnieć!)
    ├── style.css         ← Opcjonalny CSS
    └── logo.png          ← Opcjonalne obrazy
```

### 🛡️ Zabezpieczenia Custom Pages

```cpp
// ✅ DOBRZE - pełna walidacja
httpError.setCustomErrorPage(404, "/var/www/errors/404.html");
// Automatycznie sprawdzi:
// 1. Czy ścieżka nie zawiera "../" (path traversal)
// 2. Czy plik istnieje (stat)
// 3. Czy plik jest regularnym plikiem (S_ISREG)
// 4. Czy mamy uprawnienia do odczytu (access R_OK)

// ❌ ŹLE - niebezpieczna ścieżka
httpError.setCustomErrorPage(404, "/var/www/../../etc/passwd");
// Zostanie ODRZUCONE przez isValidPath()
```

### ⚠️ Obsługa błędów

```cpp
// Scenariusz 1: Plik custom page został usunięty
httpError.setCustomErrorPage(404, "/var/www/errors/404.html");
// ... później plik zostaje usunięty ...
std::string response = httpError.generateErrorResponse(404);
// → fileExists() zwróci false
// → wyloguje warning: "Custom error page no longer accessible"
// → użyje domyślnej strony

// Scenariusz 2: Plik istnieje ale jest pusty
std::string response = httpError.generateErrorResponse(500);
// → readFileContent() zwróci ""
// → wyloguje warning: "Could not read custom error page"
// → użyje domyślnej strony

// Scenariusz 3: Brak uprawnień do odczytu
// chmod 000 /var/www/errors/403.html
std::string response = httpError.generateErrorResponse(403);
// → isValidPath() zwróci false (access R_OK fails)
// → użyje domyślnej strony
```

---

## 🔥 Przykłady Rzeczywistych Scenariuszy

### Scenariusz 1: Pusty Request
```
KLIENT → (empty string)
         │
         ▼
   recivNClose() → requestStr.empty() = true
         │
         ▼
   🚨 400 Bad Request
         │
         ▼
   HTML: "Empty or malformed HTTP request."
```

### Scenariusz 2: Zbyt Duży POST
```
KLIENT → POST /upload (12KB payload)
         │
         ▼
   recivNClose() → byteRecived == 8191 (buffer full)
         │
         ▼
   🚨 413 Content Too Large
         │
         ▼
   HTML: "Request payload exceeds maximum buffer size."
```

### Scenariusz 3: Nieistniejący CGI Script
```
KLIENT → GET /cgi-bin/script.py
         │
         ▼
   router → findRoute() → CGI route found
         │
         ▼
   executeCgi() → access(scriptPath, F_OK) = -1
         │
         ▼
   🚨 404 Not Found
         │
         ▼
   HTML: "CGI script '/cgi-bin/script.py' not found."
```

### Scenariusz 4: Niedozwolona Metoda
```
KLIENT → PATCH /index.html
         │
         ▼
   Http parser → method = "PATCH"
         │
         ▼
   validateMethod() → not in [GET, POST, DELETE]
         │
         ▼
   🚨 405 Method Not Allowed
         │
         ▼
   HTML: "Method 'PATCH' is not allowed."
```

### Scenariusz 5: Exception podczas przetwarzania
```
KLIENT → GET /
         │
         ▼
   sendToClient() → Http response()
         │
         ▼
   try { response.responseBuilder() }
         │
         ▼
   ❌ Exception thrown! (np. bad_alloc)
         │
         ▼
   catch (std::exception& e)
         │
         ▼
   🚨 500 Internal Server Error
         │
         ▼
   HTML: "Internal error while processing request."
```

### Scenariusz 6: Custom Error Page - Sukces
```
KLIENT → GET /nonexistent
         │
         ▼
   router → 404 Not Found
         │
         ▼
   generateErrorResponse(404)
         │
         ▼
   hasCustomErrorPage(404) = true ✅
         │
         ▼
   getCustomErrorPagePath(404) = "/var/www/errors/404.html"
         │
         ▼
   fileExists() = true ✅
   isValidPath() = true ✅
         │
         ▼
   readFileContent() → custom HTML ✅
         │
         ▼
   🎨 Custom 404 Page
      - Własne logo
      - Branded design
      - Custom linki
         │
         ▼
   Send to client (200 bytes)
```

### Scenariusz 7: Custom Page Niedostępna
```
KLIENT → GET /forbidden
         │
         ▼
   router → 403 Forbidden
         │
         ▼
   generateErrorResponse(403)
         │
         ▼
   hasCustomErrorPage(403) = true
         │
         ▼
   getCustomErrorPagePath(403) = "/var/www/errors/403.html"
         │
         ▼
   fileExists() = false ❌
         │
         ▼
   ⚠️ std::cerr: "Custom error page no longer accessible"
         │
         ▼
   FALLBACK → generateHtmlBody(403)
         │
         ▼
   🎨 Default 403 Page (gradient background)
         │
         ▼
   Send to client
```

### Scenariusz 8: Path Traversal Attack
```
ATTACKER → setDefaultErrorPage(404, "../../../etc/passwd")
            │
            ▼
       isValidPath() checks:
            │
            ▼
       path.find("..") != npos ❌
            │
            ▼
       ⚠️ std::cerr: "Invalid or inaccessible custom error page"
            │
            ▼
       Ścieżka NIE zostaje zapisana
            │
            ▼
       ✅ Atak zablokowany!
```

---

## 🎨 Porównanie: Default vs Custom Error Page

### Default (wbudowana):
```
┌────────────────────────────────────────────┐
│         🟣 GRADIENT BACKGROUND 🟣          │
│                                            │
│              ┌──────────────┐              │
│              │     404      │              │
│              │  Not Found   │              │
│              │              │              │
│              │  [Go Home]   │              │
│              └──────────────┘              │
└────────────────────────────────────────────┘
```

### Custom (z pliku):
```
┌────────────────────────────────────────────┐
│   🎨 YOUR BRAND LOGO 🎨                    │
│                                            │
│   Oops! Page Not Found                     │
│   ━━━━━━━━━━━━━━━━━━━━                     │
│                                            │
│   We couldn't find what you're looking for.│
│                                            │
│   Try these instead:                       │
│   • Home                                   │
│   • Products                               │
│   • Contact Us                             │
│                                            │
│   [Search Site]  [Report Issue]            │
└────────────────────────────────────────────┘
```

---

**Gotowe do użycia w twoim webserverze! 🚀**

💡 **Pro tip**: Testuj custom error pages regularnie:
```bash
# Test 1: Czy plik istnieje
ls -la /var/www/errors/404.html

# Test 2: Czy mamy uprawnienia
chmod 644 /var/www/errors/*.html

# Test 3: Czy strona działa
curl http://localhost:8080/nonexistent

# Test 4: Sprawdź logi
tail -f server.log | grep "Custom error page"
```
