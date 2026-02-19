# Funzione `PARTY (BOOST)`

# SEZIONE B - DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: quando PARTY/BOOST e attivo, la macchina porta i ventilatori al massimo.
- Quando accade: appena la funzione boost viene abilitata, e resta attiva per il tempo configurato.
- Perche puo non accadere: se boost non e abilitato, se il tempo e gia scaduto, o se una logica prioritaria limita il comando.
- Eventuali ritardi voluti e relazioni con altri accessori: la durata e temporizzata; allo scadere il boost si disattiva automaticamente e il controllo torna alle logiche normali di velocita.

## B2. Comportamento normale vs percezione anomala
E normale che il boost finisca da solo senza comando manuale di stop. E normale vedere il ritorno a una velocita inferiore dopo timeout. Se la variazione e meno evidente del previsto, verificare limiti imposti da condizioni di impianto o da altri vincoli attivi.

## B3. Errori bloccanti a monte
- Logica non autorizzata: bit boost non abilitato in configurazione
- Logica in protezione: funzioni prioritarie/sicurezze che sovrascrivono la velocita
- Logica attiva ma attuatore non funzionante: comando max presente ma ventilatori non rispondono

## B4. Checklist problem solving
1. BOOST/PARTY e realmente abilitato?
2. `Set_TimeBoost` e coerente con la durata attesa?
3. Il contatore boost e gia arrivato a timeout?
4. Stato boost operativo presente nel runtime?
5. Sono attive logiche prioritarie (safety/altre regolazioni)?
6. Comando velocita massima viene effettivamente emesso?
7. Segnale elettrico verso motori presente?
8. Velocita/rpm reali aumentano?
9. Al timeout il boost si disabilita?
10. Il sistema ritorna al controllo normale?

Separazione diagnosi:
- Problema configurazione
- Problema elettrico
- Problema meccanico
- Problema installazione

## B5. Nota gestionale (facoltativa)
La modalita PARTY conferma una decisione firmware temporizzata, non garantisce da sola il risultato fluidodinamico in campo.
