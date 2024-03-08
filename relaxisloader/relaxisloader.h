/*
 * relaxisloader.h
 * Copyright (C) Carl Philipp Klemm 2023 <carl@uvos.xyz>
 *
 * relaxisloader is free software: you can redistribute it and/or modify it
 * under the terms of the lesser GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * relaxisloader is distributed in the hope that it will be useful, but
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
API for use by librelaxisloader users.
* @defgroup API User API
* This API allows you to load RelaxIS files and get the data therein
* @{
*/

struct rlxfile;

/**
 * @brief This struct represents a RelaxIS "project" containing any number of spectra.
 **/
struct rlx_project {
	int id; /**< Project id*/
	char* name; /**< Name of the project */
	time_t date; /**< Project creation time*/
};

/**
 * @brief This frees a project struct.
 *
 * This function must always be used for every singular rlx_project struct to avoid memory leaks.
 * It is safe to pass NULL to this function.
 *
 * @param proj the Project struct to be freed, or NULL.
 */
void rlx_project_free(struct rlx_project* proj);

/**
 * @brief This frees an array of project structs.
 *
 * This function must always be used for array of rlx_project structs to avoid memory leaks.
 *
 * @param proj_array The Array of project structs to be freed.
 */
void rlx_project_free_array(struct rlx_project** proj_array);

/**
 * @brief This struct is used to house a single impedance data point.
 **/
struct rlx_datapoint {
	double im; /**< Imaginary part of the measurement in Ohms*/
	double re; /**< Real part of the measurement in Ohms*/
	double omega; /**< Frequency of the measurement in rad/s*/
};

/**
 * @brief This struct is used to house an EIS spectra and associated meta-data.
 **/
struct rlx_spectra {
	int id; /**< Spectra id, also called "file" in RelaxIS*/

	struct rlx_datapoint* datapoints; /**< Data points of the spectrum*/
	size_t length; /**< Amount of data points in the spectrum*/

	char* circuit; /**< RelaxIS circuit description string*/
	bool fitted; /**< True if circuit has been fitted to spectrum*/
	int project_id; /**< Id of the project this spectrum belongs to*/
	double freq_lower_limit; /**< Lower limit of frequency range of this spectrum*/
	double freq_upper_limit; /**< Upper limit of frequency range of this spectrum*/
	time_t date_added; /**< UNIX time the spectra was added. Unfortunately, due to a deficiency in RelaxIS, the timezone of this time is unkown and the time specified assumes the project was created in a GMT+0 timezone.*/
	time_t date_fitted; /**< UNIX time the spectra was last fitted. Only valid if fitted is true. Unfortunately, due to a deficiency in RelaxIS, the timezone of this time is unkown and the time specified assumes the project was last fitted in a GMT+0 timezone.*/
};

/**
 * @brief This frees a spectra struct.
 *
 * This function must always be used for every singular rlx_spectra struct to avoid memory leaks.
 * It is safe to pass NULL to this function.
 *
 * @param spectra The spectra to be freed.
 */
void rlx_spectra_free(struct rlx_spectra* spectra);

/**
 * @brief Frees an array of spectra structs
 *
 * This function must always be used for every array of rlx_spectra structs to avoid memory leaks.
 *
 * @param spectra_array The array of spectra structs to be freed.
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
 * @param path the file system path where the file shall be opened
 * @param error if an error occurs and NULL is returned, pointer to an error string is set here,
 * owned by librelaxisloader, do not free, valid only until next call to librelaxisloader
 * @return a rlxfile struct or NULL if opening was unsuccessful, to be closed with rlx_close_file
 */
struct rlxfile* rlx_open_file(const char* path, const char** error);

void rlx_close_file(struct rlxfile* file);

/**
 * @brief Gets all the projects in a given RelaxIS file
 *
 * If this function encounters an error it will return NULL and set an error at rlx_get_errnum.
 *
 * @param file file to load projects from
 * @param length pointer to a size_t where the number of projects will be stored, or NULL
 * @return A NULL terminated array of project structs will be allocated here, to be freed with rlx_project_free_array, or NULL on error
 */
struct rlx_project** rlx_get_projects(struct rlxfile* file, size_t* length);

/**
 * @brief Loads all spectra from file in given project
 *
 * If this function encounters an error it will return NULL and set an error at rlx_get_errnum.
 *
 * @param file file to load spectra from
 * @param project project to load spectra from
 * @return A NULL terminated array of spectra structs will be allocated here, to be freed with rlx_spectra_free_array, or NULL on error
 */
struct rlx_spectra** rlx_get_all_spectra(struct rlxfile* file, const struct rlx_project* project);

/**
 * @brief Loads spectra ids that are associated with a given project
 *
 * If this function encounters an error it will return NULL and set an error at rlx_get_errnum.
 *
 * @param file file to load spectra from
 * @param project project to load spectra from
 * @param id spectra id for which to load parameters
 * @param length pointer to size_t where the number of ids will be stored or NULL
 * @return A a newly allocated array of integers with the ids, to be freed with free(), or NULL on error
 */
int* rlx_get_spectra_ids(struct rlxfile* file, const struct rlx_project* project, size_t* length);

/**
 * @brief Loads spectra with a given spectra id and project from file
 *
 * If this function encounters an error it will return NULL and set an error at rlx_get_errnum.
 *
 * @param file file to load spectra from
 * @param project project to load spectra from
 * @param id spectra id to load
 * @return spectra struct or NULL if unsuccessful, to be freed with rlx_spectra_free
 */
struct rlx_spectra* rlx_get_spectra(struct rlxfile* file, const struct rlx_project* project, int id);

/**
 * @brief Loads the parameters for a given spectra id from file
 *
 * If this function encounters an error it will return NULL and set an error at rlx_get_errnum.
 *
 * @param file file to load spectra from
 * @param project project to load spectra from
 * @param id spectra id for which to load parameters
 * @param length a pointer to a size_t where the number of parameters will be stored, or NULL
 * @return A NULL terminated array of rlx_fitparam structs will be allocated here, to be freed with rlx_fitparam_free_array, or NULL on error
 */
struct rlx_fitparam** rlx_get_fit_parameters(struct rlxfile* file, const struct rlx_project* project, int id, size_t *length);

/**
 * @brief Returns the last error returned on a file operation
 *
 * @return relaxisloader error number
 */
int rlx_get_errnum(const struct rlxfile* file);

/**
 * @brief Returns a human readable error string for a given error number
 *
 * @return Error string, static lifetime, owned by librelaxisloader do not free
 */
const char* rlx_get_errnum_str(int errnum);

/**
....
* @}
*/

#ifdef __cplusplus
}
#endif
