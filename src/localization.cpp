#include "localization.h"
#include "config.h"

Language current_language = LANG_EN;

static const LocalizedStrings strings_en = {
  "--°C", "Feels Like", "SEVEN DAY FORECAST", "FIVE DAY FORECAST", "HOURLY FORECAST",
  "Today", "Now", "am", "pm", "Noon", "Invalid hour",
  "Brightness:", "Location:", "Use °F:", "24hr:",
  "Save", "Cancel", "Close", "Location", "Reset Wi-Fi",
  "Reset", "Change Location", "Aurigae Settings",
  "City:", "Search Results", "e.g. London",
  "Wi-Fi Configuration:\n\n"
  "Please connect your\n"
  "phone or laptop to the\n"
  "temporary Wi-Fi access\n point "
  DEFAULT_CAPTIVE_SSID
  "\n"
  "to configure.\n\n"
  "If you don't see a \n"
  "configuration screen \n"
  "after connecting,\n"
  "visit http://192.168.4.1\n"
  "in your web browser.",
  "Are you sure you want to reset "
  "Wi-Fi credentials?\n\n"
  "You'll need to reconnect to the Wifi SSID " DEFAULT_CAPTIVE_SSID
  " with your phone or browser to "
  "reconfigure Wi-Fi credentials.",
  "Language:",
  {"Sun", "Mon", "Tues", "Wed", "Thu", "Fri", "Sat"},
  "MOON PHASES",
  " days to Christmas",
  "Merry Christmas!",
  "Happy New Year!",
  "New moon",
  "Waxing Crescent",
  "First Quarter",
  "Waxing Gibbous",
  "Full Moon",
  "Waning Gibbous",
  "Last Quarter",
  "Waning Crescent",
  "Hum.",
  "UV INDEX & AIR QUALITY",
  "UV Index",
  "Air Quality",
  "Low", "Moderate", "High", "Very High", "Extreme",
  "Good", "Fair", "Moderate", "Poor", "Very Poor"
};

static const LocalizedStrings strings_it = {
  "--°C", "Percepiti", "PREVISIONE 7 GIORNI", "PREVISIONE 5 GIORNI", "PREVISIONE ORARIA",
  "Oggi", "Ora", "am", "pm", "Mezzogiorno", "Ora non valida",
  "Luminosità:", "Località:", "Usa °F:", "24hr:",
  "Salva", "Cancella", "Chiudi", "Località", "Reset Wi-Fi",
  "Reset", "Cambia località", "Impostazioni Aurigae",
  "Città:", "Risultati ricerca", "e.g. Londra",
  "Configurazione WiFi:\n\n"
  "Connetti il tuo\n"
  "smartphone o PC al\n"
  "punto di accesso \n WiFi temporaneo \n"
  DEFAULT_CAPTIVE_SSID
  "\n"
  "per configurare.\n\n"
  "Se non vedi una \n"
  "schermata di configurazione \n"
  "dopo la connessione,\n"
  "visita http://192.168.4.1\n"
  "dal tuo browser.",
  "Sei sicuro di voler fare il reset "
  "delle credenziali WiFi?\n\n"
  "Dovrai riconnetterti al Wifi SSID " DEFAULT_CAPTIVE_SSID
  " con il tuo smartphone o PC "
  "per riconfigurare le credenziali WiFi.",
  "Lingua:",
  {"Dom", "Lun", "Mar", "Mer", "Gio", "Ven", "Sab"},
  "FASI LUNARI",
  " giorni a Natale",
  "Buon Natale!",
  "Buon Anno!",
  "Luna nuova",
  "Luna crescente",
  "Primo quarto",
  "Gibbosa crescente",
  "Luna piena",
  "Gibbosa calante",
  "Ultimo quarto",
  "Luna calante",
  "Umi.",
  "INDICE UV E QUALITÀ ARIA",
  "Indice UV",
  "Qualità Aria",
  "Basso", "Moderato", "Alto", "Molto Alto", "Estremo",
  "Buona", "Discreta", "Moderata", "Scarsa", "Molto Scarsa"
};

static const LocalizedStrings strings_es = {
  "--°C", "Sensación", "PRONÓSTICO 7 DÍAS", "PRONÓSTICO 5 DÍAS", "PRONÓSTICO POR HORAS",
  "Hoy", "Ahora", "am", "pm", "Mediodía", "Hora inválida",
  "Brillo:", "Ubicación:", "Usar °F:", "24h:",
  "Guardar", "Cancelar", "Cerrar", "Ubicación", "Wi-Fi",
  "Restablecer", "Cambiar Ubicación", "Configuración Aurigae",
  "Ciudad:", "Resultados de Búsqueda", "ej. Madrid",
  "Configuración Wi-Fi:\n\n"
  "Conecte su teléfono\n"
  "o portátil al punto de\n"
  "acceso Wi-Fi temporal\n"
  DEFAULT_CAPTIVE_SSID
  "\n"
  "para configurar.\n\n"
  "Si no ve una pantalla\n"
  "de configuración después\n"
  "de conectarse, visite\n"
  "http://192.168.4.1\n"
  "en su navegador.",
  "¿Está seguro de que desea\n"
  "restablecer las credenciales\n"
  "Wi-Fi?\n\n"
  "Deberá reconectarse al SSID " DEFAULT_CAPTIVE_SSID
  " con su teléfono o navegador\n"
  "para reconfigurar las\n"
  "credenciales Wi-Fi.",
  "Idioma:",
  {"Dom", "Lun", "Mar", "Mié", "Jue", "Vie", "Sáb"},
  "FASES DE LA LUNA",
  " días hasta Navidad",
  "¡Feliz Navidad!",
  "¡Feliz Año Nuevo!",
  "Luna nueva",
  "Creciente",
  "Cuarto creciente",
  "Gibosa creciente",
  "Luna llena",
  "Gibosa menguante",
  "Cuarto menguante",
  "Menguante",
  "Hum.",
  "ÍNDICE UV Y CALIDAD DEL AIRE",
  "Índice UV",
  "Calidad del Aire",
  "Bajo", "Moderado", "Alto", "Muy Alto", "Extremo",
  "Buena", "Aceptable", "Moderada", "Mala", "Muy Mala"
};

static const LocalizedStrings strings_de = {
  "--°C", "Gefühlt", "7-TAGE VORHERSAGE", "5-TAGE VORHERSAGE", "STÜNDLICHE VORHERSAGE",
  "Heute", "Jetzt", "", "", "Mittag", "Ungültige Stunde",
  "Helligkeit:", "Standort:", "°F:", "24h:",
  "Speichern", "Abbrechen", "Schließen", "Standort", "Wi-Fi",
  "Zurücksetzen", "Standort ändern", "Aurigae Einstellungen",
  "Stadt:", "Suchergebnisse", "z.B. Berlin",
  "Wi-Fi Konfiguration:\n\n"
  "Verbinden Sie Ihr Telefon\n"
  "oder Laptop mit dem\n"
  "temporären Wi-Fi\n"
  "Zugangspunkt "
  DEFAULT_CAPTIVE_SSID
  "\n"
  "zum Konfigurieren.\n\n"
  "Wenn Sie keinen\n"
  "Konfigurationsbildschirm\n"
  "sehen, besuchen Sie\n"
  "http://192.168.4.1\n"
  "in Ihrem Browser.",
  "Sind Sie sicher, dass Sie\n"
  "die Wi-Fi Zugangsdaten\n"
  "zurücksetzen möchten?\n\n"
  "Sie müssen sich erneut mit\n"
  "der SSID " DEFAULT_CAPTIVE_SSID
  " verbinden, um die\n"
  "Wi-Fi Zugangsdaten\n"
  "neu zu konfigurieren.",
  "Sprache:",
  {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"},
  "Mondphasen",
  " Tage bis Weihnachten",
  "Frohe Weihnachten!",
  "Frohes Neues Jahr!",
  "Neumond",
  "Zunehmende Sichel",
  "Erstes Viertel",
  "Zunehmender Dreiviertelmond",
  "Vollmond",
  "Abnehmender Dreiviertelmond",
  "Letztes Viertel",
  "Abnehmende Sichel",
  "Feuch.", //humidity
  "UV-INDEX & LUFTQUALITÄT",
  "UV-Index",
  "Luftqualität",
  "Niedrig", "Mäßig", "Hoch", "Sehr Hoch", "Extrem",
  "Gut", "Mittelmäßig", "Mäßig", "Schlecht", "Sehr Schlecht"
};

static const LocalizedStrings strings_fr = {
  "--°C", "Ressenti", "PRÉVISIONS 7 JOURS", "PRÉVISIONS 5 JOURS", "PRÉVISIONS HORAIRES",
  "Aujourd'hui", "Maintenant", "h", "h", "Midi", "Heure invalide",
  "Luminosité:", "Lieu:", "Utiliser °F:", "24h:",
  "Sauvegarder", "Annuler", "Fermer", "Lieu", "Wi-Fi",
  "Réinitialiser", "Changer de lieu", "Paramètres Aurigae",
  "Ville:", "Résultats de recherche", "ex. Paris",
  "Configuration Wi-Fi:\n\n"
  "Connectez votre téléphone\n"
  "ou ordinateur portable au\n"
  "point d'accès Wi-Fi\n"
  "temporaire "
  DEFAULT_CAPTIVE_SSID
  "\n"
  "pour configurer.\n\n"
  "Si vous ne voyez pas\n"
  "d'écran de configuration\n"
  "après connexion, visitez\n"
  "http://192.168.4.1\n"
  "dans votre navigateur.",
  "Êtes-vous sûr de vouloir\n"
  "réinitialiser les\n"
  "identifiants Wi-Fi?\n\n"
  "Vous devrez vous reconnecter\n"
  "au SSID " DEFAULT_CAPTIVE_SSID
  " avec votre téléphone ou\n"
  "navigateur pour reconfigurer\n"
  "les identifiants Wi-Fi.",
  "Langue:",
  {"Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"},
  "phases de la lune",
  " jours avant Noël",
  "Joyeux Noël!",
  "Bonne Année!",
  "Nouvelle lune",
  "Croissant croissant",
  "Premier quartier",
  "Gibbeuse croissante",
  "Pleine lune",
  "Gibbeuse décroissante",
  "Dernier quartier",
  "Croissant décroissant",
  "Hum.",
  "INDICE UV & QUALITÉ DE L'AIR",
  "Indice UV",
  "Qualité de l'air",
  "Faible", "Modéré", "Élevé", "Très Élevé", "Extrême",
  "Bonne", "Correcte", "Moyenne", "Mauvaise", "Très Mauvaise"
};

const LocalizedStrings* get_strings() {
  switch (current_language) {
    case LANG_ES: return &strings_es;
    case LANG_DE: return &strings_de;
    case LANG_FR: return &strings_fr;
    case LANG_IT: return &strings_it;
    default: return &strings_en;
  }
}
