/******************************************************************************************************************************/
/* ABLS-AGENT-GPIOD/gpiod.c  Gestion de l'agent GPIOD                                                                        */
/* Projet Abls-Habitat                   Gestion d'habitat                                                15.09.2026 12:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * gpiod.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sebastien LEFEVRE
 *
 * ABLS-AGENT-GPIOD is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ABLS-AGENT-GPIOD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ABLS-AGENT-GPIOD; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #include <stdlib.h>
 #include <string.h>

 #include "abls-gpiod.h"

 struct ABLS_AGENT *Agent = NULL;
 struct ABLS_GPIOD_VARS *Agent_vars = NULL;

/******************************************************************************************************************************/
/* Gpiod_release_line_resources: libère les ressources de configuration d'une ligne GPIO                                      */
/* Entrée: la configuration de requête, de ligne et les paramètres de ligne                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Gpiod_release_line_resources ( struct gpiod_request_config *req_cfg,
                                            struct gpiod_line_config *line_cfg,
                                            struct gpiod_line_settings *settings )
  { if (req_cfg) gpiod_request_config_free ( req_cfg );
    if (line_cfg) gpiod_line_config_free ( line_cfg );
    if (settings) gpiod_line_settings_free ( settings );
  }
/******************************************************************************************************************************/
/* Charger_un_gpio: charge et configure une ligne GPIO depuis la configuration JSON                                           */
/* Entrée: le tableau, l'index, l'élément JSON et les données utilisateur                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Charger_un_gpio ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { gint num = Json_get_int ( element, "num" );
    if (num < 0 || num >= Agent_vars->num_lines)
     { Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_ERR, "%s: num %d is out of range (0..%d)",
             Json_get_string ( element, "agent_acronyme" ), num, Agent_vars->num_lines - 1 );
       return;
     }

    Json_add_bool ( element, "need_sync", TRUE );
    Agent_vars->lignes[num].element = element;
    Agent_vars->lignes[num].mode_inout = Json_get_int ( element, "mode_inout" );
    Agent_vars->lignes[num].mode_activelow = Json_get_int ( element, "mode_activelow" );
    Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_INFO,
          "Chargement du GPIO%02d en mode_inout %d, mode_activelow=%d",
          num, Agent_vars->lignes[num].mode_inout, Agent_vars->lignes[num].mode_activelow );

    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (!settings)
     { Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_ERR, "GPIO%02d: gpiod_line_settings_new error", num );
       return;
     }

    gpiod_line_settings_set_direction ( settings,
                                        (Agent_vars->lignes[num].mode_inout == 0 ? GPIOD_LINE_DIRECTION_INPUT
                                                                                 : GPIOD_LINE_DIRECTION_OUTPUT) );

    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    if (!line_cfg)
     { Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_ERR, "GPIO%02d: gpiod_line_config_new error", num );
       Gpiod_release_line_resources ( NULL, NULL, settings );
       return;
     }

    unsigned int offset = num;
    gint ret = gpiod_line_config_add_line_settings ( line_cfg, &offset, 1, settings );
    if (ret)
     { Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_ERR, "GPIO%02d: gpiod_line_config_add_line_settings error", num );
       Gpiod_release_line_resources ( NULL, line_cfg, settings );
       return;
     }

    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    if (req_cfg) gpiod_request_config_set_consumer ( req_cfg, "ABLS Agent GPIOD" );

    Agent_vars->lignes[num].gpio_ligne = gpiod_chip_request_lines ( Agent_vars->chip, req_cfg, line_cfg );
    if (!Agent_vars->lignes[num].gpio_ligne)
     { Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_ERR, "GPIO%02d: gpiod_chip_request_lines error", num );
       Gpiod_release_line_resources ( req_cfg, line_cfg, settings );
       return;
     }

    Gpiod_release_line_resources ( req_cfg, line_cfg, settings );

    if (Agent_vars->lignes[num].mode_inout == 0)
     { gchar chaine[256];
       g_snprintf ( chaine, sizeof(chaine), "/usr/bin/raspi-gpio set %d ip pd", num );
       system ( chaine );
       Agent_vars->lignes[num].etat = gpiod_line_request_get_value ( Agent_vars->lignes[num].gpio_ligne, num );
     }
    else
     { Agent_vars->lignes[num].etat = (Agent_vars->lignes[num].mode_activelow ? TRUE : FALSE);
       gpiod_line_request_set_value ( Agent_vars->lignes[num].gpio_ligne, num, Agent_vars->lignes[num].etat );
     }
    Json_add_int ( element, "etat", Agent_vars->lignes[num].etat );
  }
#ifdef bouh
/******************************************************************************************************************************/
/* Gpiod_sync_all_inputs: synchronise les états des entrées GPIO vers MQTT                                                    */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Gpiod_sync_all_inputs ( void )
  { for (gint cpt = 0; cpt < Agent_vars->num_lines; cpt++)
     { if (Agent_vars->lignes[cpt].gpio_ligne && Agent_vars->lignes[cpt].mode_inout == 0)
        { Agent_vars->lignes[cpt].etat = gpiod_line_request_get_value ( Agent_vars->lignes[cpt].gpio_ligne, cpt );
          Mqtt_Send_DI ( Agent, Agent_vars->lignes[cpt].element, Agent_vars->lignes[cpt].etat );
        }
     }
  }
#endif
/******************************************************************************************************************************/
/* Gpiod_cleanup: libère les lignes GPIO et les ressources de l'agent                                                         */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Gpiod_cleanup ( void )
  { if (!Agent_vars) return;
    for (gint cpt = 0; cpt < Agent_vars->num_lines; cpt++)
     { if (Agent_vars->lignes && Agent_vars->lignes[cpt].gpio_ligne)
        { gpiod_line_request_release ( Agent_vars->lignes[cpt].gpio_ligne ); }
     }

    if (Agent_vars->lignes) g_free ( Agent_vars->lignes );
    Agent_vars->lignes = NULL;
    if (Agent_vars->chip) gpiod_chip_close ( Agent_vars->chip );
    Agent_vars->chip = NULL;
  }
/******************************************************************************************************************************/
/* main: Prend en charge l'agent                                                                                              */
/* Entrée: argc, argv                                                                                                         */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 gint main ( gint argc, gchar *argv[] )
  { Agent = Agent_init ( argv[0], "gpiod", ABLS_AGENT_GPIOD_VERSION, sizeof(struct ABLS_GPIOD_VARS), argc, argv );
    Agent_vars = Agent_get_vars ( Agent );

    Agent_vars->chip = gpiod_chip_open ( "/dev/gpiochip0" );
    if (!Agent_vars->chip)
     { Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_ERR, "Error while loading chip 'gpiochip0'" );
       Agent_end ( Agent );
     }
    else Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_NOTICE, "Chip 'gpiochip0' loaded" );

    struct gpiod_chip_info *info = gpiod_chip_get_info ( Agent_vars->chip );
    if (!info)
     { Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_ERR, "gpiod_chip_get_info failed" );
       Gpiod_cleanup();
       Agent_end ( Agent );
     }

    Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_NOTICE, "%s [%s] %d lines",
          gpiod_chip_info_get_name(info), gpiod_chip_info_get_label(info), gpiod_chip_info_get_num_lines(info) );
    Agent_vars->num_lines = gpiod_chip_info_get_num_lines ( info );
    gpiod_chip_info_free ( info );

    if (Agent_vars->num_lines > GPIOD_MAX_LINE) Agent_vars->num_lines = GPIOD_MAX_LINE;
    Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_INFO, "Found %d lines", Agent_vars->num_lines );

    JsonNode *RootNode = Json_create();
    if (RootNode)
     { Json_add_string ( RootNode, "agent_tech_id", Agent_get_tech_id ( Agent ) );
       Json_add_int    ( RootNode, "nbr_lignes", Agent_vars->num_lines );
       JsonNode *API_result = Http_Post_to_global_API ( Agent, "/run/gpiod/add/io", RootNode );
       if (API_result) Json_unref ( API_result );
       Json_unref ( RootNode );
     }

    Agent_vars->lignes = g_try_malloc0 ( sizeof(struct ABLS_GPIOD_LINE) * Agent_vars->num_lines );
    if (!Agent_vars->lignes)
     { Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_ALERT, "Memory Error while loading lignes" );
       Gpiod_cleanup();
       Agent_end ( Agent );
     }

    Agent_config_foreach_array_element ( Agent, "IO", Charger_un_gpio, Agent );
    Agent_send_comm_to_master ( Agent, TRUE );
    Agent_is_ready ( Agent );

    while (Agent_is_running ( Agent ))
     { Agent_loop ( Agent );

/*+--------------------------------------------------------------Ecoute du master --------------------------------------------*/
       JsonNode *mqtt_local_message;
       while ( (mqtt_local_message = Agent_get_mqtt_local_message ( Agent ) ) != NULL )
        { if (Mqtt_topic_is ( mqtt_local_message, 3, "SET_DO", Agent_get_tech_id ( Agent ), "+" ))
           { const gchar *msg_agent_acronyme = Mqtt_get_topic_lvl ( mqtt_local_message, 2 );
             gchar *msg_tech_id        = Json_get_string ( mqtt_local_message, "tech_id" );
             gchar *msg_acronyme       = Json_get_string ( mqtt_local_message, "acronyme" );
             gboolean etat             = Json_get_bool ( mqtt_local_message, "etat" );

             for (gint num = 0; num < Agent_vars->num_lines; num++)
              { if ( Agent_vars->lignes[num].gpio_ligne && Agent_vars->lignes[num].mode_inout != 0 &&
                     !strcasecmp ( Json_get_string ( Agent_vars->lignes[num].element, "agent_acronyme" ), msg_agent_acronyme ) )
                 { Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_NOTICE, "SET_DO '%s:%s'/'%s:%s'=%d",
                         Agent_get_tech_id ( Agent ), msg_agent_acronyme, msg_tech_id, msg_acronyme, etat );
                         Agent_vars->lignes[num].etat = etat;
                         Json_add_bool ( Agent_vars->lignes[num].element, "etat", etat );
                         gpiod_line_request_set_value ( Agent_vars->lignes[num].gpio_ligne, num, etat );
                   break;
                 }
              }
           }
          Json_unref ( mqtt_local_message );
        }

/*+--------------------------------------------------------------Ecoute de l'API ---------------------------------------------*/
       JsonNode *mqtt_api_message;
       while ( (mqtt_api_message = Agent_get_mqtt_api_message ( Agent ) ) != NULL )
        { if ( Mqtt_topic_is ( mqtt_api_message, 4, "+", "AGENT", Agent_get_tech_id ( Agent ), "TEST" ) )
           { Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_NOTICE, "Agent Test from API." ); }
           /* Gpiod_sync_all_inputs(); a revoir : mettre ca dans agent-libs, basé sur le tableau "IOs" avec les classes DI/AI */
          Json_unref ( mqtt_api_message ); }
        }

    Gpiod_cleanup();
    Agent_end ( Agent );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/