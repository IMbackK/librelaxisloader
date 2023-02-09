/*
 * relaxisloader.h
 * Copyright (C) Carl Philipp Klemm 2023 <carl@uvos.xyz>
 *
 * relaxisloader.h is free software: you can redistribute it and/or modify it
 * under the terms of the lesser GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * relaxisloader.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the lesser GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
Api for use by librelaxisloader users.
* @defgroup API User API
* This api allows you to load relaxis files and get the data therein
* @{
*/

struct rxfile;

struct rlx_project {
	int id; /**< Project id*/
	char* name; /**< Name of the project */
	time_t date; /**< Project creation time*/
};

/**
 * @brief Frees a project struct
 *
 * @param proj project struct to be freed, or NULL
 */
void rlx_project_free(struct rlx_project* proj);

/**
 * @brief Frees an array of project structs
 *
 * @param proj_array array of project structs to be freed
 */
void rlx_project_free_array(struct rlx_project** proj_array);

struct rlx_datapoint {
	double im; /**< imaginary part of the mesurement in Ohms*/
	double re; /**< real part of the mesurement in Ohms*/
	double omega; /**< freqency of the mesurement in rad/s*/
};

struct rlx_spectra {
	int id; /**< Spectra id, also called "file" in relaxis*/

	struct rlx_datapoint* datapoints; /**< datapoints of the spectrum*/
	size_t length; /**< Amount of datapoints in the spectrum*/

	char* circuit; /**< Relaxis circuit descriptian string*/
	bool fitted; /**< True if circuit has been fitted to spectrum*/
	int project_id; /**< Id of the project this spectrum belongs to*/
	double freq_lower_limit; /**< Lower limit of freqency range of this spctrum*/
	double freq_upper_limit; /**< Upper limit of freqency range of this spctrum*/
	time_t date_added;
	time_t date_fitted;
};

/**
 * @brief Frees a spectra struct
 *
 * @param spectra spectra to be freed
 */
void rlx_spectra_free(struct rlx_spectra* spectra);

/**
 * @brief Frees an array of spectra structs
 *
 * @param spectra_array array of spectra structs to be freed
 */
void rlx_spectra_free_array(struct rlx_spectra** spectra_array);

struct rlx_fitparam {
	int spectra_id;
	int p_index;
	char* name;
	double value;
	double error;
	double lower_limit;
	double upper_limit;
};

/**
 * @brief Frees a fitparam struct
 *
 * @param fitparam fitparam struct to be freed
 */
void rlx_fitparam_free(struct rlx_fitparam* param);

/**
 * @brief Frees an array of fitparam structs
 *
 * @param param_array array of fitparam structs to be freed
 */
void rlx_fitparam_free_array(struct rlx_fitparam** param_array);

/**
 * @brief Frees a project struct
 *
 * @param path the filesystem path where the file shal be opened
 * @param error if an error occures and NULL is retuned, pointer to an error string is set here,
 * owned by librelaxisloader, do not free, valid only untill next call to librelaxisloader
 * @return a rxfile struct or NULL if opening was unsucessfull, to be closed with rlx_close_file
 */
struct rxfile* rlx_open_file(const char* path, const char** error);

void rlx_close_file(struct rxfile* file);

/**
 * @brief Gets all the projects in a given RelaxIS file
 *
 * @param file file to load projects from
 * @param length pointer to a size_t where the number of projects will be stored, or NULL
 * @return A NULL terminated array of project structs will be allocated here, to be freed with rlx_project_free_array, or NULL on error
 */
struct rlx_project** rlx_get_projects(struct rxfile* file, size_t* length);

/**
 * @brief Loads all spectra from file in given proeject
 *
 * @param file file to load spectra from
 * @param project project to load spectra from
 * @return A NULL terminated array of spectra structs will be allocated here, to be freed with rlx_spectra_free_array, or NULL on error
 */
struct rlx_spectra** rlx_get_all_spectra(struct rxfile* file, const struct rlx_project* project);

/**
 * @brief Loads specra ids that are assoicated with a given project
 *
 * @param file file to load spectra from
 * @param project project to load spectra from
 * @param id spectra id for wich to load parameters
 * @param length pointer to size_t where the number of ids will be stored or NULL
 * @return A NULL terminated array of fitparam structs will be allocated here, to be freed with rlx_fitparam_free_array, or NULL on error
 */
int* rlx_get_spectra_ids(struct rxfile* file, const struct rlx_project* project, size_t* length);

/**
 * @brief Loads spectra with a given specra id and project from file
 *
 * @param file file to load spectra from
 * @param project project to load spectra from
 * @param id spectra id to load
 * @return spectra struct or NULL if unsucessful, to be freed with rlx_spectra_free
 */
struct rlx_spectra* rlx_get_spectra(struct rxfile* file, const struct rlx_project* project, int id);

/**
 * @brief Loads the parameters for a given spectra id from file
 *
 * @param file file to load spectra from
 * @param project project to load spectra from
 * @param id spectra id for wich to load parameters
 * @param length a pointer to a size_t where the number of parameters will be stored, or NULL
 * @return A NULL terminated array of fitparam structs will be allocated here, to be freed with rlx_fitparam_free_array, or NULL on error
 */
struct rlx_fitparam** rlx_get_fit_parameters(struct rxfile* file, const struct rlx_project* project, int id, size_t *length);

/**
 * @brief Returns the last error retuned on a file operation
 *
 * @return relaxiloader error number
 */
int rlx_get_errnum(const struct rxfile* file);

/**
 * @brief Returns a human reable error string for a given error number
 *
 * @return Error string, owend by librelaxisloader do not free
 */
const char* rlx_get_errnum_str(int errnum);

/**
....
* @}
*/

#ifdef __cplusplus
}
#endif
