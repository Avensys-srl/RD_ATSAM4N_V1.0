# Funzione `allarmi_firmware` (spiegazione operativa completa)

# SEZIONE B - DOCUMENTAZIONE NON TECNICA (OPERATIVA / CAMPO)

## B1. Racconto operativo
- Cosa accade nel sistema: il controllo segnala allarmi quando rileva guasti su sonde, accessori, motori, comunicazioni o stati macchina. Gli allarmi vengono mostrati su interfaccia/telecontrollo e possono attivare limitazioni funzionali.
- Quando accade: durante il funzionamento normale, a ogni ciclo di controllo, appena una condizione di errore supera i controlli previsti.
- Perche puo non accadere: errore intermittente sotto soglia temporale, sensore non configurato, funzione accessoria non installata, oppure guasto reale non ancora acquisito.
- Eventuali ritardi voluti e relazioni con altri accessori: alcuni allarmi hanno una persistenza intenzionale prima della segnalazione per evitare falsi positivi; diversi allarmi dipendono da presenza/configurazione accessori e dai moduli esterni collegati.

Cosa vede l'utente:
- Codice/allarme attivo in diagnostica.
- Possibile riduzione prestazioni, blocco funzione o comportamento in protezione.
- Rientro automatico quando la causa scompare (dove previsto) oppure necessita di reset/manutenzione nei casi persistenti.

## B2. Comportamento normale vs percezione anomala
- E normale vedere allarmi di link su accessori opzionali non presenti se la configurazione non e coerente con l'installato.
- E normale che allarmi intermittenti non compaiano subito: la logica puo richiedere persistenza temporale.
- E normale che la macchina continui a funzionare con allarmi non bloccanti (esempio: filtro sporco), ma con messaggio attivo.
- Falsa diagnosi tipica: interpretare un allarme di comunicazione come guasto hardware del sensore, quando il problema reale e cablaggio/alimentazione bus.

## B3. Errori bloccanti a monte
Classificazione operativa:
- Logica non autorizzata: funzione disabilitata o accessorio non previsto, con conseguente mancata attivazione dei relativi controlli.
- Logica in protezione: allarmi critici attivi (motori, batterie, bypass, sonde principali) che forzano limitazioni o blocchi.
- Logica attiva ma attuatore non funzionante: comando presente ma risposta fisica assente (esempio motore alimentato senza tachimetro valido).

Categorie complete allarmi presenti nel firmware:
- Accessori idronici/espansione: EHD/HWD, CWD, PEHD/PHWD, DXD.
- Qualita aria e sonde ambiente: CO2, RH, VOC, DSC.
- Sensori pressione/portata: PCAP, PCAF, DPP, FLW1/FLW2.
- Sonde temperatura: Fresh, Return, Supply, Exhaust, EXT1..EXT4.
- Motori ventilatori: assenza alimentazione e assenza impulsi/tachimetro su rami mandata e ripresa.
- Funzioni sistema: bypass, batteria, filtro, memoria EEPROM, fault controllo remoto/KTS.
- Moduli esterni dedicati: AWP, EBPD, SSR, EBP2.
- Eventi safety/smoke mode: stati forzati ventilazione (OFF/MAX/solo estrazione/solo mandata).

## B4. Checklist problem solving
1. Quale allarme specifico e attivo (codice esatto)?
2. L'allarme riguarda un dispositivo realmente installato e configurato?
3. Alimentazione dispositivo presente e stabile?
4. Cablaggio e polarita corretti?
5. Collegamento comunicazione integro (bus/link)?
6. Misura sensore coerente con il valore reale in campo?
7. Timer/persistenze diagnostiche rispettate (attendere tempo tecnico)?
8. La macchina e in protezione generale per un altro allarme a monte?
9. Con comando attivo, esiste riscontro elettrico sull'uscita?
10. Con riscontro elettrico corretto, il componente fisico reagisce?

Separazione diagnosi:
- Problema configurazione: mappa I/O o accessorio non coerente.
- Problema elettrico: alimentazione, fusibili, connessioni, bus.
- Problema meccanico: blocco ventilatore, bypass, valvole, organi in movimento.
- Problema installazione: sensore non idoneo, posa errata, disturbi EMC, messa a terra.

## B5. Nota gestionale (facoltativa)
La presenza di un allarme indica una condizione rilevata dal controllo, ma non identifica da sola la responsabilita operativa. La responsabilita va definita con verifica tecnica in campo su configurazione, cablaggio e componente fisico.
