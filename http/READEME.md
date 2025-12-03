# HTTP - Dokumentacja

### Konstruktor
```cpp
Http(std::string &rawRequest, ServerConfig serverData)
```
- `rawRequest` - surowy request HTTP z nagłówkami i opcjonalnym body
- `serverData` - konfiguracja serwera (dozwolone metody, max body size, etc.)

### Metody publiczne
```cpp
bool getIsError() const
```
Zwraca `true` jeśli podczas przetwarzania wystąpił błąd.

```cpp
int getStatusCode() const
```
Zwraca kod statusu HTTP odpowiedzi (200, 404, 405, 500, etc.).

```cpp
std::string getResponse() const
```
Zwraca pełną odpowiedź HTTP jako string (nagłówki + body), gotową do wysłania.

## Obsługiwane metody HTTP
- **GET** - pobieranie zasobów
- **POST** - tworzenie/wysyłanie danych
- **DELETE** - usuwanie zasobów

## Przykładowe kody statusu
- **200** - OK (request wykonany pomyślnie)
- **404** - Not Found (zasób nie istnieje)
- **405** - Method Not Allowed (metoda niedozwolona dla tego zasobu)
- **413** - Payload Too Large (body przekracza limit)
- **500** - Internal Server Error (błąd serwera)

## Uwagi implementacyjne
- Klasa automatycznie parsuje nagłówki i body z surowego requestu
- Body nie jest kopiowane - używany jest wskaźnik do oryginału (optymalizacja pamięci)
- Konfiguracja z `ServerConfig` określa:
  - Dozwolone metody HTTP
  - Maksymalny rozmiar body
  - Ścieżki do zasobów
  - Ustawienia CGI
- W przypadku błędu parsowania lub przetwarzania, obiekt zwraca odpowiedni kod błędu i generuje stronę HTML z opisem problemu
