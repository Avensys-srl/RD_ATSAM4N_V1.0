# Funzione `filter_settings` (gestione filtri)

# SEZIONE B - DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: il sistema monitora lo stato filtri e segnala quando e richiesta manutenzione/pulizia.
- Quando accade: in continuo, con ciclo a 1 secondo, usando ore funzionamento e/o pressostato DPP.
- Perche puo non accadere: controllo filtri disabilitato, sensore non calibrato/disponibile, o logica in fallback.
- Eventuali ritardi voluti e relazioni con altri accessori: in DPP la verifica e periodica (tipicamente ogni 200 ore) e puo prendere temporaneamente il controllo velocita motori per test.

## B2. Comportamento normale vs percezione anomala
E normale che l'allarme filtro non compaia subito: dipende da timer o finestre di verifica DPP. E normale che in assenza calibrazione DPP il sistema torni a modalita temporale (DPS). Durante verifica/calibrazione DPP e normale osservare variazioni pilotate di velocita motori.

## B3. Errori bloccanti a monte
- Logica non autorizzata: `ENAB_CTRL_FILTER` disabilitato
- Logica in protezione: richiesta DPP non valida o calibrazione assente
- Logica attiva ma attuatore/sensore non funzionante: DPP non leggibile, risultato filtro non attendibile

## B4. Checklist problem solving
1. Controllo filtri abilitato?
2. Modalita attiva: DPS o DPP?
3. Ore funzionamento e ultimo reset filtro coerenti?
4. Parametro manutenzione `gg_manut_Filter` corretto?
5. In DPP: calibrazione presente (`DPP_Calibrationvalue`)?
6. Richiesta verifica/calibrazione DPP attiva o chiusa?
7. Pressostato legge valori coerenti?
8. `ALM_FILTER` presente/assente coerentemente con la condizione?
9. Durante DPP check i motori vengono pilotati come previsto?
10. Dopo pulizia/reset, lo stato torna normale?

Separazione diagnosi:
- Problema configurazione
- Problema elettrico
- Problema meccanico
- Problema installazione

## B5. Nota gestionale (facoltativa)
La gestione firmware dei filter settings supporta la manutenzione, ma la diagnosi finale richiede sempre verifica fisica del filtro e del sensore differenziale in campo.
