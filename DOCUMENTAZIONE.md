# PROGETTO: Sistema di Illuminazione Intelligente per Smart Home

## Autori
**Benny e Giulia**

---

## Obiettivo

Realizzare un **sistema di illuminazione intelligente e automatizzato** che migliori il comfort domestico, aumenti la sicurezza e riduca gli sprechi energetici, attraverso l'integrazione di sensori, attuatori e sistemi di controllo automatico.

Il progetto utilizza una tastiera Keypad 4×4 per l'accesso sicuro, evitando l'utilizzo del monitor seriale e rendendo il sistema più realistico e professionale, simile ai veri sistemi domotici utilizzati nelle abitazioni intelligenti moderne.

---

## Componenti Utilizzati

| Componente | Quantità | Funzione |
|-----------|----------|---------|
| Arduino Uno | 1 | Microcontrollore principale |
| Breadboard | 1 | Base per i circuiti |
| Cavo USB | 1 | Alimentazione e programmazione |
| LED | 4 | Illuminazione (principale + scale) |
| Resistenze | 5 | Protezione circuiti |
| Cavi jumper | 15–20 | Connessioni |
| Buzzer | 1 | Feedback sonoro |
| Display OLED | 1 | Interfaccia utente visiva |
| Fotoresistenza LDR | 1 | Rilevamento luminosità ambiente |
| Sensore di movimento PIR | 1 | Rilevamento presenza persone |
| Sensore di suono KY-037 | 1 | Rilevamento battiti di mani/rumori |
| Servo motore SG90 | 1 | Controllo apertura porta |
| Tastierino Keypad 4×4 | 1 | Inserimento PIN |
| Modulo Relay | - | Opzionale (controllo carichi elevati) |

---

## Descrizione del Sistema

Il sistema gestisce l'illuminazione domestica attraverso modalità **automatiche e controllate**, integrando:

- **Sicurezza**: Accesso protetto tramite PIN
- **Automazione**: Sensori che controllano automaticamente le luci
- **Efficienza energetica**: Spegnimento automatico in presenza di luce naturale
- **Interazione intelligente**: Controllo tramite movimenti e suoni

Questo sistema rappresenta una soluzione completa di **domotica residenziale**, capace di simulare il funzionamento reale di una moderna smart home.

---

## Avvio del Sistema

All'accensione, il display OLED mostra:

```
┌─────────────────────┐
│ Inserire il PIN      │
│ per aprire la porta  │
└─────────────────────┘
```

Il sistema rimane in attesa dell'inserimento del codice tramite il tastierino Keypad 4×4. Questa fase di autenticazione rappresenta la sicurezza iniziale del sistema.

---

## Funzionamento del Sistema

### 1. Accesso con PIN tramite Keypad 4×4

L'utente inserisce un codice numerico (default: `1234`) utilizzando il tastierino Keypad 4×4.

**Se il codice è corretto:**
- Display mostra: `"Codice corretto"` + `"Accesso consentito"`
- Il servo motore simula l'apertura della porta (90°)
- La luce principale si accende
- Il buzzer riproduce una melodia positiva di conferma (1000Hz, 1300Hz, 1600Hz)
- Il sistema passa alla modalità sbloccata

**Se il codice è errato:**
- Display mostra: `"Codice errato"` + `"Riprova"`
- Il servo motore resta fermo (0°)
- Le luci rimangono spente
- Il buzzer emette un suono grave di errore (350Hz, 250Hz)
- Il PIN viene azzerato e il sistema ritorna alla schermata iniziale

**Timeout PIN**: Se l'utente non inserisce nulla per più di 30 secondi, il PIN viene automaticamente cancellato e il sistema richiede un nuovo inserimento.

---

### 2. Illuminazione Automatica delle Scale

Il **sensore PIR** rileva la presenza di persone tramite il movimento corporeo.

**Quando viene rilevato un passaggio (PIR = HIGH):**
- I LED delle scale si accendono automaticamente
- Illuminazione progressiva: STEP_LED1 si accende prima, poi STEP_LED2 (ritardo di 150ms)
- Il tempo di rilevamento viene aggiornato
- **Prerequisiti**: Sistema sbloccato (`unlocked = true`) e condizioni di buio (`isDark() = true`)

**In assenza di movimento:**
- Dopo 5 secondi di inattività (SCALE_TIMEOUT = 5000ms), i LED si spengono automaticamente
- Questo meccanismo riduce i consumi energetici

**Realismo**: L'accensione progressiva dei LED simula l'illuminazione intelligente delle scale moderne.

---

### 3. Controllo tramite Suono

Il **sensore KY-037** rileva battiti di mani o rumori improvvisi.

**Quando viene rilevato un suono:**
- Il buzzer riproduce una breve melodia (900Hz, 1100Hz)
- Se l'ambiente è al buio, la luce principale **alterna il suo stato** (on/off)
- Il display mostra: `"Suono rilevato"` + `"Luce principale ON/OFF"`
- Visualizzazione per 700ms (non-bloccante)
- **Debounce**: Timeout di 1 secondo per evitare falsi positivi

**Prerequisiti**: Sistema sbloccato (`unlocked = true`)

Questa funzione permette un'interazione semplice e immediata tramite gesto/suono.

---

### 4. Risparmio Energetico Automatico

La **fotoresistenza LDR** rileva la quantità di luce naturale (threshold = 500).

**In presenza di luce diurna (LDR > 500):**
- Le luci artificiali rimangono spente automaticamente
- Il sistema disabilita la luce principale e quella delle scale
- Riduzione del consumo energetico durante il giorno

**In condizioni di buio (LDR < 500):**
- Il sistema abilita automaticamente l'accensione delle luci se appropriate
- La luce principale può accendersi dopo un accesso PIN
- Le scale si illuminano quando il PIR rileva movimento

Questo meccanismo consente di ridurre il consumo energetico e migliorare l'efficienza della casa intelligente.

---

## Logica del Sistema

Il sistema opera integrando **più sensori contemporaneamente** secondo precise priorità:

```
┌─────────────────────────────────────┐
│     LOOP PRINCIPALE (Non-bloccante) │
├─────────────────────────────────────┤
│ 1. handleKeypad()                   │
│    → Accesso PIN (con timeout 30s)  │
│                                     │
│ 2. handleKeypadDisplay()            │
│    → Gestione visualizzazione       │
│                                     │
│ 3. handlePIR()                      │
│    → Controllo luci scale (5s)      │
│                                     │
│ 4. handleSoundSensor()              │
│    → Toggle luce + melodia          │
│                                     │
│ 5. handleSoundSensorDisplay()       │
│    → Gestione visualizzazione       │
│                                     │
│ 6. handleEnergySaving()             │
│    → Spegnimento in base a LDR      │
└─────────────────────────────────────┘
```

**Priorità funzionali:**

1. **Keypad** → Controlla l'accesso sicuro all'abitazione
2. **PIR** → Gestisce l'illuminazione automatica delle scale
3. **LDR** → Regola il risparmio energetico
4. **Sensore sonoro** → Permette l'interazione tramite rumore
5. **Display, Buzzer, Servo** → Forniscono feedback visivi e sonori

---

## Assegnazione Pin

| Pin | Tipo | Componente | Funzione |
|-----|------|-----------|----------|
| 2 | Digitale | Keypad Col 4 | Colonna tastiera |
| 3 | Digitale | Keypad Col 3 | Colonna tastiera |
| 4 | Digitale | Keypad Col 2 | Colonna tastiera |
| 5 | Digitale | Keypad Col 1 | Colonna tastiera |
| 6 | Digitale | Keypad Row 4 | Riga tastiera |
| 7 | Digitale | Keypad Row 3 | Riga tastiera |
| 8 | Digitale | Keypad Row 2 | Riga tastiera |
| 9 | Digitale | Keypad Row 1 | Riga tastiera |
| 10 | Digitale | PIR Sensor | Rilevamento movimento |
| 11 | Digitale | Sound Sensor | Rilevamento suono |
| 12 | Digitale | Servo Motor | Controllo porta |
| 13 | Digitale | Buzzer | Feedback sonoro |
| A0 | Analogico | LDR | Rilevamento luminosità |
| A1 | Analogico | Main LED | Luce principale |
| A2 | Analogico | Step LED 1 | Luce scala 1 |
| A3 | Analogico | Step LED 2 | Luce scala 2 |

---

## Innovazioni Tecniche

Il progetto implementa diverse innovazioni:

### Programmazione Non-Bloccante
- Utilizzo di `millis()` invece di `delay()` per evitare blocchi
- Due funzioni dedicate (`handleKeypadDisplay()`, `handleSoundSensorDisplay()`) gestiscono i timeout
- Il sistema rimane completamente responsivo durante tutte le operazioni

### Timeout Intelligenti
- **PIN Timeout** (30 secondi): Cancella automaticamente il PIN se non confermato
- **Scale Timeout** (5 secondi): Spegne le luci delle scale se nessun movimento
- **Sound Debounce** (1 secondo): Evita falsi positivi dal sensore sonoro
- **Sound Display** (700ms): Mostra il messaggio brevemente

### Sistema Serial per Debug
- `Serial.begin(9600)` per diagnostica tramite monitor seriale
- Messaggi di avvio, inizializzazione e stato sistema
- Utilità per debugging e monitoraggio in tempo reale

### Accensione Progressiva
- LED scale non si accendono istantaneamente
- STEP_LED1 si accende, poi STEP_LED2 dopo 150ms
- Simula l'illuminazione intelligente moderna

### Sicurezza Multilivello
- PIN numerico a 4 cifre
- Feedback visivo e sonoro per tentativi errati
- Servo motore controlla l'accesso fisico
- Timeout automatico per sessioni inattive

---

## Struttura del Codice

Il file `main.h` è organizzato in sezioni logiche:

1. **Include librerie** → Comunicazione I2C, display OLED, Keypad, Servo
2. **Definizioni OLED** → Dimensioni display
3. **Assegnazione Pin** → Mapping hardware
4. **Configurazione Keypad** → Matrice 4×4 e connessioni
5. **Servo** → Angoli apertura/chiusura porta
6. **Variabili di stato** → Tracciamento dello stato del sistema
7. **Funzioni Display** → Gestione visualizzazione OLED
8. **Funzioni Buzzer** → Melodie di feedback
9. **Funzioni Porta** → Controllo servo motore
10. **Funzioni Illuminazione** → Controllo LED
11. **Funzioni Sensori** → Lettura sensori
12. **Logica PIN** → Gestione autenticazione
13. **Handler eventi** → Processamento continuo
14. **Setup** → Inizializzazione sistema
15. **Loop** → Ciclo principale non-bloccante

---

## Aspetti Didattici

Questo progetto è ideale per un liceo scientifico perché:

### Competenze Informatiche
- Programmazione C/C++ per microcontrollori
- Strutture dati (String, array, boolean)
- Funzioni e modularità del codice
- Gestione della memoria e delle risorse

### Competenze Elettroniche
- Circuiti digitali e analogici
- Sensori e attuatori
- Protocolli di comunicazione (I2C, PWM)
- Breadboard e cablaggio

### Competenze Sistemistiche
- Macchina a stati (locked/unlocked)
- Priorità e sequenze di esecuzione
- Timing non-bloccante
- Debugging e monitoraggio

### Competenze Progettuali
- Design dell'interfaccia utente
- Scelta dei componenti appropriati
- Integrazione di sistemi complessi
- Documentazione tecnica

---

## Analisi Funzionale

### Diagramma di Stato

```
        ┌─────────────┐
        │   LOCKED    │
        │  (Avvio)    │
        └──────┬──────┘
               │
          [PIN corretto]
               ↓
        ┌──────────────┐
        │   UNLOCKED   │
        │   (Accesso)  │
        └──────┬───────┘
               │
         [Inattività 30s] OR [Timeout sessione]
               ↓
        ┌──────────────┐
        │   RE-LOCKED  │
        │ (Reset sistema)
        └──────────────┘
```

### Tabella di Verità dei Sensori

| PIR | LDR | Accesso | Azione |
|-----|-----|---------|--------|
| 0 | 0 | ✓ | Scale OFF |
| 1 | 0 | ✓ | Scale ON (progressivo) |
| 0 | 1 | ✓ | Qualsiasi luce OFF |
| 1 | 1 | ✓ | Dipende da stato manuale |
| X | X | ✗ | Tutte OFF |

---

## Risultati Attesi

**Funzionalità Completate:**

- Autenticazione tramite PIN a 4 cifre
- Controllo servo motore per porta
- Illuminazione principale intelligente
- Illuminazione progressiva scale
- Rilevamento movimento PIR
- Controllo tramite suono
- Risparmio energetico LDR
- Feedback sonoro (buzzer)
- Display OLED
- Programmazione non-bloccante
- Timeout intelligenti
- Serial debugging  

---

## Possibili Sviluppi Futuri

- Aggiungere modulo Relay per controllare carichi maggiori
- Implementare memoria EEPROM per salvataggio PIN
- Aggiungere sensore temperatura/umidità
- Implementare scheduling orario
- Collegamento a Wi-Fi e cloud
- App mobile per controllo remoto
- Sistema di registrazione eventi
- Sensore di fumo/gas per sicurezza

---

## Conclusioni

Il progetto realizza un **sistema di illuminazione intelligente completo**, capace di combinare:

- **Sicurezza** → Accesso protetto tramite PIN
- **Comfort** → Illuminazione automatica e progressiva
- **Efficienza energetica** → Spegnimento automatico in base alla luce naturale
- **Interattività** → Controllo tramite gesti e suoni

Attraverso l'utilizzo di sensori, attuatori e sistemi di controllo automatico, il sistema dimostra come l'**elettronica e la programmazione** possano essere applicate concretamente nella gestione efficiente degli ambienti domestici.

L'utilizzo del Keypad 4×4 rende il progetto **più professionale, realistico e vicino alle tecnologie utilizzate nelle vere abitazioni intelligenti**, evitando la dipendenza dal monitor seriale e creando un'esperienza utente autentica.

---

**Data creazione**: Maggio 2026  
**Materia**: Sistemi e Reti / Informatica  
**Livello**: Liceo Scientifico (Classe Quinta)
