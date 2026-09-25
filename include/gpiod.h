/******************************************************************************************************************************/
/* ABLS-AGENT-GPIOD/include/gpiod.h   Déclaration structure interne du module GPIOD                                          */
/* Projet Abls-Habitat                   Gestion d'habitat                                                15.09.2026 12:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * gpiod.h
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

#ifndef _ABLS_GPIOD_H_
 #define _ABLS_GPIOD_H_

 #include <abls-agent-libs/abls-agent-libs.h>
 #include <gpiod.h>

 #define GPIOD_MAX_LINE 28

 struct ABLS_GPIOD_LINE
  { gboolean etat;
    gboolean mode_inout;
    gboolean mode_activelow;
    struct gpiod_line_request *gpio_ligne;
    JsonNode *element;
  };

 struct ABLS_GPIOD_VARS
  { struct gpiod_chip *chip;
    gint num_lines;
    struct ABLS_GPIOD_LINE *lignes;
  };

 extern struct ABLS_AGENT *Agent;
 extern struct ABLS_GPIOD_VARS *Agent_vars;

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
