# Lista errori e allarmi firmware (guida utente finale)

Questa lista contiene i codici allarme numerici reali del firmware (esclusi i codici riservati non usati in campo).
Per ogni codice trovi:
- Nota: la codifica utente `BB-b` e l'offset `00-0 = not identified error` sono indicazioni fornite dal committente.
- Significato: cosa sta succedendo
- Cosa fare: controlli e azioni consigliate

## Codice errore utente e codifica firmware

Per tua indicazione, i codici mostrati all'utente sono in formato `BB-b` con offset di +1 rispetto all'ID firmware.
- `00-0`: riservato a `not identified error`
- Dal primo allarme firmware in poi, il codice utente scala di una posizione
- Sotto ogni errore trovi una nota con il codice firmware originale (`E###`)

| Codice utente | ID firmware | Byte firmware | Bit firmware |
|---|---:|---:|---:|
| `00-0` | n/a | n/a | n/a |
| `00-1` | 0 | 0 | 0 |
| `00-2` | 1 | 0 | 1 |
| `00-3` | 2 | 0 | 2 |
| `00-4` | 3 | 0 | 3 |
| `00-5` | 4 | 0 | 4 |
| `00-6` | 5 | 0 | 5 |
| `00-7` | 6 | 0 | 6 |
| `01-0` | 7 | 0 | 7 |
| `01-1` | 8 | 1 | 0 |
| `01-2` | 9 | 1 | 1 |
| `01-3` | 10 | 1 | 2 |
| `01-4` | 11 | 1 | 3 |
| `01-5` | 12 | 1 | 4 |
| `01-6` | 13 | 1 | 5 |
| `02-0` | 15 | 1 | 7 |
| `02-1` | 16 | 2 | 0 |
| `02-2` | 17 | 2 | 1 |
| `02-3` | 18 | 2 | 2 |
| `02-4` | 19 | 2 | 3 |
| `02-5` | 20 | 2 | 4 |
| `02-6` | 21 | 2 | 5 |
| `02-7` | 22 | 2 | 6 |
| `03-0` | 23 | 2 | 7 |
| `03-1` | 24 | 3 | 0 |
| `03-2` | 25 | 3 | 1 |
| `03-3` | 26 | 3 | 2 |
| `03-4` | 27 | 3 | 3 |
| `03-6` | 29 | 3 | 5 |
| `03-7` | 30 | 3 | 6 |
| `04-3` | 34 | 4 | 2 |
| `04-4` | 35 | 4 | 3 |
| `04-6` | 37 | 4 | 5 |
| `04-7` | 38 | 4 | 6 |
| `05-3` | 42 | 5 | 2 |
| `05-4` | 43 | 5 | 3 |
| `05-7` | 46 | 5 | 6 |
| `06-0` | 47 | 5 | 7 |
| `06-1` | 48 | 6 | 0 |
| `06-2` | 49 | 6 | 1 |
| `06-4` | 51 | 6 | 3 |
| `06-5` | 52 | 6 | 4 |
| `06-7` | 54 | 6 | 6 |
| `07-0` | 55 | 6 | 7 |
| `07-1` | 56 | 7 | 0 |
| `07-2` | 57 | 7 | 1 |
| `07-3` | 58 | 7 | 2 |
| `07-4` | 59 | 7 | 3 |
| `07-7` | 62 | 7 | 6 |
| `08-0` | 63 | 7 | 7 |
| `08-1` | 64 | 8 | 0 |
| `08-2` | 65 | 8 | 1 |
| `08-3` | 66 | 8 | 2 |
| `08-5` | 68 | 8 | 4 |
| `08-6` | 69 | 8 | 5 |
| `08-7` | 70 | 8 | 6 |
| `09-1` | 72 | 9 | 0 |
| `09-2` | 73 | 9 | 1 |
| `09-3` | 74 | 9 | 2 |
| `09-5` | 76 | 9 | 4 |
| `09-6` | 77 | 9 | 5 |
| `09-7` | 78 | 9 | 6 |
| `10-1` | 80 | 10 | 0 |
| `10-2` | 81 | 10 | 1 |
| `10-3` | 82 | 10 | 2 |
| `10-4` | 83 | 10 | 3 |
| `10-5` | 84 | 10 | 4 |
| `10-6` | 85 | 10 | 5 |
| `10-7` | 86 | 10 | 6 |
| `11-1` | 88 | 11 | 0 |
| `11-2` | 89 | 11 | 1 |
| `11-3` | 90 | 11 | 2 |
| `11-4` | 91 | 11 | 3 |
| `11-5` | 92 | 11 | 4 |
| `11-6` | 93 | 11 | 5 |
| `11-7` | 94 | 11 | 6 |
| `12-0` | 95 | 11 | 7 |
| `12-1` | 96 | 12 | 0 |
| `12-2` | 97 | 12 | 1 |
| `12-3` | 98 | 12 | 2 |
| `12-4` | 99 | 12 | 3 |
| `12-5` | 100 | 12 | 4 |
| `12-6` | 101 | 12 | 5 |
| `12-7` | 102 | 12 | 6 |
| `13-0` | 103 | 12 | 7 |
| `13-1` | 104 | 13 | 0 |
| `13-2` | 105 | 13 | 1 |
| `13-3` | 106 | 13 | 2 |
| `13-4` | 107 | 13 | 3 |
| `13-5` | 108 | 13 | 4 |
| `13-6` | 109 | 13 | 5 |

---
## Accessori EHD/HWD
- `00-1`
  - Nota firmware: `E000`
  - Significato: sonda temperatura accessorio EHD/HWD guasta o scollegata.
  - Cosa fare: verificare cablaggio sonda, connettori, integrita sonda; sostituire la sonda se fuori specifica.
- `00-2`
  - Nota firmware: `E001`
  - Significato: temperatura acqua in zona pericolosa.
  - Cosa fare: controllare circuito idraulico, portata acqua, valvola e pompa; verificare setpoint.
- `00-3`
  - Nota firmware: `E002`
  - Significato: temperatura acqua fuori range operativo.
  - Cosa fare: verificare temperatura reale acqua, taratura sonda e configurazione limiti.
- `00-4`
  - Nota firmware: `E003`
  - Significato: guasto elettrico accessorio EHD/HWD.
  - Cosa fare: controllare alimentazione, protezioni, cablaggio potenza/comando.
- `00-5`
  - Nota firmware: `E004`
  - Significato: temperatura aria associata all'accessorio fuori range.
  - Cosa fare: controllare sonde aria, filtri, portata ventilazione e setpoint.
- `00-6`
  - Nota firmware: `E005`
  - Significato: taglia accessorio non coerente con unita.
  - Cosa fare: verificare configurazione modello/taglia in setup.
- `00-7`
  - Nota firmware: `E006`
  - Significato: anomalia pressione/portata sul circuito accessorio.
  - Cosa fare: controllare pressioni, perdite, valvole e presenza aria nel circuito.
- `01-0`
  - Nota firmware: `E007`
  - Significato: perdita comunicazione con accessorio EHD/HWD.
  - Cosa fare: controllare bus, indirizzi, alimentazione modulo e terminazioni.

## Accessori CWD
- `01-1`
  - Nota firmware: `E008`
  - Significato: sonda CWD guasta/scollegata.
  - Cosa fare: controllare e sostituire sonda.
- `01-2`
  - Nota firmware: `E009`
  - Significato: acqua in zona pericolo.
  - Cosa fare: verificare circuito idraulico e setpoint.
- `01-3`
  - Nota firmware: `E010`
  - Significato: acqua fuori range.
  - Cosa fare: verificare temperatura reale e limiti impostati.
- `01-4`
  - Nota firmware: `E011`
  - Significato: guasto elettrico CWD.
  - Cosa fare: controllare alimentazione/protezioni/cablaggi.
- `01-5`
  - Nota firmware: `E012`
  - Significato: aria fuori range in gestione CWD.
  - Cosa fare: verificare sonde e portata aria.
- `01-6`
  - Nota firmware: `E013`
  - Significato: taglia accessorio non coerente.
  - Cosa fare: ricontrollare configurazione macchina.
- `02-0`
  - Nota firmware: `E015`
  - Significato: link CWD assente.
  - Cosa fare: controllare comunicazione e alimentazione modulo.

## Accessori PEHD/PHWD
- `02-1`
  - Nota firmware: `E016`
  - Significato: sonda PEHD guasta.
  - Cosa fare: controllo cablaggio e sostituzione sonda.
- `02-2`
  - Nota firmware: `E017`
  - Significato: acqua in zona pericolosa.
  - Cosa fare: verificare impianto idraulico.
- `02-3`
  - Nota firmware: `E018`
  - Significato: acqua fuori range.
  - Cosa fare: verificare limiti e lettura sonda.
- `02-4`
  - Nota firmware: `E019`
  - Significato: guasto elettrico accessorio.
  - Cosa fare: controllo alimentazione/protezioni.
- `02-5`
  - Nota firmware: `E020`
  - Significato: aria fuori range.
  - Cosa fare: verificare sonde e ventilazione.
- `02-6`
  - Nota firmware: `E021`
  - Significato: taglia non coerente.
  - Cosa fare: verificare configurazione.
- `02-7`
  - Nota firmware: `E022`
  - Significato: pressione/portata non corretta.
  - Cosa fare: controllare pressione e pompa.
- `03-0`
  - Nota firmware: `E023`
  - Significato: assenza comunicazione accessorio.
  - Cosa fare: verificare bus e cablaggio.

## Sonde CO2 / DSC
- `03-1`
  - Nota firmware: `E024`
  - Significato: dispositivo DSC guasto.
  - Cosa fare: verificare alimentazione, collegamento e sostituire dispositivo se necessario.
- `03-2`
  - Nota firmware: `E025`
  - Significato: comunicazione DSC assente.
  - Cosa fare: controllare bus, indirizzamento e continuita linea.
- `03-3`
  - Nota firmware: `E026`
  - Significato: sonda CO2 esterna 1 guasta.
  - Cosa fare: controllare sensore, cablaggio e sostituzione.
- `03-4`
  - Nota firmware: `E027`
  - Significato: link sonda CO2 esterna 1 assente.
  - Cosa fare: verificare comunicazione/indirizzo.
- `03-6`
  - Nota firmware: `E029`
  - Significato: sonda CO2 esterna 2 guasta.
  - Cosa fare: stessi controlli della sonda CO2 EX1.
- `03-7`
  - Nota firmware: `E030`
  - Significato: link sonda CO2 esterna 2 assente.
  - Cosa fare: verificare cablaggio bus/indirizzo.

## Sonde RH (umidita)
- `04-3`
  - Nota firmware: `E034`
  - Significato: sonda RH esterna 1 guasta.
  - Cosa fare: controllo sensore e cablaggio.
- `04-4`
  - Nota firmware: `E035`
  - Significato: link RH esterna 1 assente.
  - Cosa fare: controllo comunicazione bus.
- `04-6`
  - Nota firmware: `E037`
  - Significato: sonda RH esterna 2 guasta.
  - Cosa fare: controllo sensore/cablaggio.
- `04-7`
  - Nota firmware: `E038`
  - Significato: link RH esterna 2 assente.
  - Cosa fare: verifica linea e indirizzo.

## VOC / DXD
- `05-3`
  - Nota firmware: `E042`
  - Significato: sonda VOC guasta.
  - Cosa fare: verificare alimentazione sensore, cablaggio e sostituzione.
- `05-4`
  - Nota firmware: `E043`
  - Significato: assenza comunicazione sonda VOC.
  - Cosa fare: controllare bus e configurazione sonda.
- `05-7`
  - Nota firmware: `E046`
  - Significato: anomalia compressore modulo DXD.
  - Cosa fare: verificare compressore, protezioni, pressostati e consenso comando.
- `06-0`
  - Nota firmware: `E047`
  - Significato: modulo DXD non comunicante.
  - Cosa fare: controllare alimentazione e rete di comunicazione modulo.

## Sensori pressione / portata
- `06-1`
  - Nota firmware: `E048`
  - Significato: sensore pressione PCAP guasto.
  - Cosa fare: verificare sensore e cablaggio.
- `06-2`
  - Nota firmware: `E049`
  - Significato: link sensore PCAP assente.
  - Cosa fare: verificare linea comunicazione.
- `06-4`
  - Nota firmware: `E051`
  - Significato: sensore pressione PCAF guasto.
  - Cosa fare: verificare sensore/cablaggio.
- `06-5`
  - Nota firmware: `E052`
  - Significato: link PCAF assente.
  - Cosa fare: verifica bus/indirizzo.
- `06-7`
  - Nota firmware: `E054`
  - Significato: sensore differenziale pressione guasto.
  - Cosa fare: verificare presa pressione e sensore.
- `07-0`
  - Nota firmware: `E055`
  - Significato: link DPP assente.
  - Cosa fare: controllare collegamento e alimentazione.

## Fire mode / AWP
- `07-1`
  - Nota firmware: `E056`
  - Significato: ventilazione forzata in stato OFF da logica smoke.
  - Cosa fare: verificare ingresso antincendio e priorita sicurezza.
- `07-2`
  - Nota firmware: `E057`
  - Significato: ventilazione forzata a massimo per modalita smoke.
  - Cosa fare: verificare comando antincendio e logiche emergenza.
- `07-3`
  - Nota firmware: `E058`
  - Significato: solo estrazione attiva in modalita smoke.
  - Cosa fare: verificare schema antincendio e consensi ventilatori.
- `07-4`
  - Nota firmware: `E059`
  - Significato: solo mandata attiva in modalita smoke.
  - Cosa fare: verificare configurazione emergenza e cablaggi.
- `07-7`
  - Nota firmware: `E062`
  - Significato: modulo/sensore AWP guasto.
  - Cosa fare: verificare dispositivo, alimentazione e sostituzione.
- `08-0`
  - Nota firmware: `E063`
  - Significato: link AWP assente.
  - Cosa fare: verificare bus comunicazione e indirizzo.

## Motori ramo R (ripresa)
- `08-1`
  - Nota firmware: `E064`
  - Significato: motore R1 senza alimentazione.
  - Cosa fare: controllare potenza, fusibili, driver.
- `08-2`
  - Nota firmware: `E065`
  - Significato: motore R2 senza alimentazione.
  - Cosa fare: come sopra.
- `08-3`
  - Nota firmware: `E066`
  - Significato: motore R3 senza alimentazione.
  - Cosa fare: come sopra.
- `08-5`
  - Nota firmware: `E068`
  - Significato: motore R1 senza tachimetro/pulsazioni.
  - Cosa fare: verificare feedback e rotazione reale.
- `08-6`
  - Nota firmware: `E069`
  - Significato: motore R2 senza pulsazioni.
  - Cosa fare: controllo sensore feedback/driver.
- `08-7`
  - Nota firmware: `E070`
  - Significato: motore R3 senza pulsazioni.
  - Cosa fare: come sopra.

## Motori ramo F (mandata)
- `09-1`
  - Nota firmware: `E072`
  - Significato: motore F1 senza alimentazione.
  - Cosa fare: controllare linea potenza e driver.
- `09-2`
  - Nota firmware: `E073`
  - Significato: motore F2 senza alimentazione.
  - Cosa fare: come sopra.
- `09-3`
  - Nota firmware: `E074`
  - Significato: motore F3 senza alimentazione.
  - Cosa fare: come sopra.
- `09-5`
  - Nota firmware: `E076`
  - Significato: motore F1 senza tachimetro/pulsazioni.
  - Cosa fare: verificare feedback e blocchi meccanici.
- `09-6`
  - Nota firmware: `E077`
  - Significato: motore F2 senza pulsazioni.
  - Cosa fare: verifica feedback/driver.
- `09-7`
  - Nota firmware: `E078`
  - Significato: motore F3 senza pulsazioni.
  - Cosa fare: verifica feedback/driver.

## Sistema generale
- `10-1`
  - Nota firmware: `E080`
  - Significato: errore controllo ventilatori.
  - Cosa fare: verificare driver fan, feedback giri, configurazione motori.
- `10-2`
  - Nota firmware: `E081`
  - Significato: errore controllo remoto/board esterna.
  - Cosa fare: verificare comunicazione remota e stato scheda.
- `10-3`
  - Nota firmware: `E082`
  - Significato: anomalia batteria (modulo trattamento).
  - Cosa fare: verificare consenso, sonde collegate e componenti batteria.
- `10-4`
  - Nota firmware: `E083`
  - Significato: bypass non funzionante o non raggiunge posizione richiesta.
  - Cosa fare: verificare attuatore bypass, finecorsa, cablaggio e ostruzioni meccaniche.
- `10-5`
  - Nota firmware: `E084`
  - Significato: errore memoria EEPROM.
  - Cosa fare: spegnere/riaccendere, verificare persistenza parametri, ripristino configurazione e assistenza tecnica.
- `10-6`
  - Nota firmware: `E085`
  - Significato: filtro sporco/intasato o manutenzione filtro richiesta.
  - Cosa fare: ispezionare e sostituire/pulire filtro, poi azzerare contatore manutenzione.
- `10-7`
  - Nota firmware: `E086`
  - Significato: errore comunicazione/servizio KTS.
  - Cosa fare: verificare connessione KTS, parametri rete/seriale e riavvio interfaccia.

## Sonde temperatura e ingressi esterni
- `11-1`
  - Nota firmware: `E088`
  - Significato: sonda aria esterna (fresh) guasta.
  - Cosa fare: verificare sonda/cablaggio.
- `11-2`
  - Nota firmware: `E089`
  - Significato: sonda ritorno guasta.
  - Cosa fare: verifica cablaggio e sostituzione.
- `11-3`
  - Nota firmware: `E090`
  - Significato: sonda mandata guasta.
  - Cosa fare: controllo posizione e connessioni.
- `11-4`
  - Nota firmware: `E091`
  - Significato: sonda espulsione guasta.
  - Cosa fare: verifica sonda e cablaggio.
- `11-5`
  - Nota firmware: `E092`
  - Significato: link EXT1 assente.
  - Cosa fare: verificare linea collegamento EXT1.
- `11-6`
  - Nota firmware: `E093`
  - Significato: dispositivo/sensore EXT1 guasto.
  - Cosa fare: controllare dispositivo e sostituire.
- `11-7`
  - Nota firmware: `E094`
  - Significato: link EXT2 assente.
  - Cosa fare: verifica cablaggio e indirizzo.
- `12-0`
  - Nota firmware: `E095`
  - Significato: dispositivo/sensore EXT2 guasto.
  - Cosa fare: verifica e sostituzione.
- `12-1`
  - Nota firmware: `E096`
  - Significato: link EXT3 assente.
  - Cosa fare: controllo linea/bus.
- `12-2`
  - Nota firmware: `E097`
  - Significato: dispositivo EXT3 guasto.
  - Cosa fare: verifica dispositivo.
- `12-3`
  - Nota firmware: `E098`
  - Significato: link EXT4 assente.
  - Cosa fare: controllo cablaggio.
- `12-4`
  - Nota firmware: `E099`
  - Significato: dispositivo EXT4 guasto.
  - Cosa fare: verifica/sostituzione.
- `12-5`
  - Nota firmware: `E100`
  - Significato: link sensore flusso 1 assente.
  - Cosa fare: verificare connessione sensore.
- `12-6`
  - Nota firmware: `E101`
  - Significato: sensore flusso 1 guasto.
  - Cosa fare: verifica sensore e circuito aria/acqua collegato.
- `12-7`
  - Nota firmware: `E102`
  - Significato: link sensore flusso 2 assente.
  - Cosa fare: controllare comunicazione.
- `13-0`
  - Nota firmware: `E103`
  - Significato: sensore flusso 2 guasto.
  - Cosa fare: verifica e sostituzione.

## Moduli EBPD / SSR / EBP2
- `13-1`
  - Nota firmware: `E104`
  - Significato: link modulo EBPD assente.
  - Cosa fare: verificare bus e alimentazione modulo.
- `13-2`
  - Nota firmware: `E105`
  - Significato: modulo EBPD guasto.
  - Cosa fare: diagnosi modulo e sostituzione.
- `13-3`
  - Nota firmware: `E106`
  - Significato: link SSR assente.
  - Cosa fare: verificare comunicazione/connessione SSR.
- `13-4`
  - Nota firmware: `E107`
  - Significato: SSR guasto.
  - Cosa fare: controllare comando, carico e sostituire SSR.
- `13-5`
  - Nota firmware: `E108`
  - Significato: link modulo EBP2 assente.
  - Cosa fare: verificare cablaggio e indirizzo.
- `13-6`
  - Nota firmware: `E109`
  - Significato: modulo EBP2 guasto.
  - Cosa fare: diagnosi modulo e sostituzione.

## Procedura rapida consigliata (sempre valida)
1. Annotare codice allarme preciso.
2. Verificare se il componente e realmente installato e abilitato in configurazione.
3. Controllare alimentazione e cablaggio del componente.
4. Controllare comunicazione (link/bus/indirizzo).
5. Verificare stato fisico del componente (blocco meccanico, sensore guasto, filtro intasato).
6. Dopo intervento, cancellare/rientrare allarme e verificare che non ricompaia.

Se un allarme ritorna subito dopo il reset, la causa fisica non e stata rimossa.



