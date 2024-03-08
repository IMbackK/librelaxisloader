/*
 * relaxisloader
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

#include "relaxisloader.h"

#include <stdlib.h>
#include <math.h>
#include <sqlite3.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

struct rlxfile
{
	int error;
	sqlite3 *db;
};

/**
 * @brief Frees a project struct
 *
 * @param proj project struct to be freed, or NULL
 */
void rlx_project_free(struct rlx_project* proj)
{
	if(!proj)
		return;
	if(proj->name)
		free(proj->name);
	free(proj);
}

void rlx_project_free_array(struct rlx_project** proj)
{
	struct rlx_project** firstproj = proj;
	while(*proj) {
		rlx_project_free(*proj);
		++proj;
	}
	free(firstproj);
}

void rlx_spectra_free(struct rlx_spectra* specta)
{
	if(!specta)
		return;

	if(specta->circuit)
		free(specta->circuit);
	free(specta->datapoints);
	free(specta);
}

void rlx_spectra_free_array(struct rlx_spectra** specta)
{
	struct rlx_spectra** firstspectra = specta;
	while(*specta) {
		rlx_spectra_free(*specta);
		++specta;
	}
	free(firstspectra);
}

void rlx_fitparam_free(struct rlx_fitparam* param)
{
	if(!param)
		return;
	if(param->name)
		free(param->name);
	free(param);
}

void rlx_fitparam_free_array(struct rlx_fitparam** param)
{
	struct rlx_fitparam** firstparam = param;
	while(*param) {
		rlx_fitparam_free(*param);
		++param;
	}
	free(firstparam);
}

struct rlxfile* rlx_open_file(const char* path, const char** error)
{
	struct rlxfile *file = calloc(1, sizeof(*file));
	int ret = sqlite3_open_v2(path, &file->db, SQLITE_OPEN_READONLY, NULL);
	if(!(ret == SQLITE_OK || ret == SQLITE_DONE)) {
		if(error)
			*error = sqlite3_errstr(ret);
		free(file);
		return NULL;
	}

	const char *req = "SELECT Value FROM Properties WHERE Name=\"DatabaseFormat\"";
	sqlite3_stmt *ppStmt;
	ret = sqlite3_prepare_v2(file->db, req, strlen(req), &ppStmt, NULL);
	if(ret != SQLITE_OK) {
		if(error)
			*error = "Unable to read file version";
		return NULL;
	}

	ret = sqlite3_step(ppStmt);
	if((ret != SQLITE_OK && ret != SQLITE_DONE && ret != SQLITE_ROW) || sqlite3_column_count(ppStmt) != 1) {
		if(error)
			*error = "Unable to read file version, field missing";
		return NULL;
	}

	int version = sqlite3_column_int(ppStmt, 0);
	if(version != 1) {
		if(error)
			*error = "Unsupported file version";
		return NULL;
	}

	if(error)
		*error = NULL;

	sqlite3_finalize(ppStmt);
	return file;
}

void rlx_close_file(struct rlxfile* file)
{
	sqlite3_close(file->db);
	free(file);
}

struct rlx_project** rlx_get_projects(struct rlxfile* file, size_t* length)
{
	char **table;
	int rows;
	int cols;
	char *error;
	int ret = sqlite3_get_table(file->db, "SELECT ID,NAME,DATE FROM Projects", &table, &rows, &cols, &error);

	if(ret != SQLITE_OK) {
		file->error = ret;
		free(error);
		if(length)
			*length = 0;
		return NULL;
	}
	rows++;
	assert(cols == 3);

	struct rlx_project **projects = malloc(sizeof(*projects)*(rows));
	assert(projects);

	if(length)
		*length = rows-1;

	for(int i = 1; i < rows; ++i) {
		projects[i-1] = malloc(sizeof(struct rlx_project));
		assert(projects[i-1]);
		projects[i-1]->name = rlx_strdup(table[i*cols+1]);
		int ret = sscanf(table[i*cols], "%d", &projects[i-1]->id);
		assert(ret == 1);
		projects[i-1]->date = rlx_str_to_time(table[i*cols+2]);
	}
	projects[rows-1] = NULL;

	sqlite3_free_table(table);
	return projects;
}

static struct rlx_datapoint* rlx_get_datapoints(struct rlxfile* file, int id, size_t *length)
{
	char **table;
	int rows;
	int cols;
	char *error;
	char *req = rlx_alloc_printf("SELECT frequency,zreal,zimag FROM Datapoints WHERE file_id=%d", id);
	int ret = sqlite3_get_table(file->db, req, &table, &rows, &cols, &error);
	free(req);
	++rows;
	if(ret != SQLITE_OK) {
		file->error = ret;
		free(error);
		if(length)
			*length = 0;
		return NULL;
	}
	assert(cols == 3);

	if(rows < 2) {
		file->error = -100;
		sqlite3_free_table(table);
		return NULL;
	}

	if(length)
		*length = rows-1;
	struct rlx_datapoint *out = malloc(sizeof(*out)*(rows-1));

	for(int i = 1; i < rows; ++i) {
		int ret = sscanf(table[i*cols], "%lf", &out[i-1].omega);
		assert(ret == 1);
		out[i-1].omega *= 2*M_PI;
		ret = sscanf(table[i*cols+1], "%lf", &out[i-1].re);
		assert(ret == 1);
		ret = sscanf(table[i*cols+2], "%lf", &out[i-1].im);
		assert(ret == 1);
	}
	sqlite3_free_table(table);
	return out;
}

struct rlx_spectra* rlx_get_spectra(struct rlxfile* file, const struct rlx_project* project, int id)
{
	char **table;
	int rows;
	int cols;
	char *error;
	char *req = rlx_alloc_printf(
		"SELECT groupname,fitted,lowfreqlimit,highfreqlimit,dateadded,datefitted FROM Files WHERE project_id=%d AND ID=%d",
		project->id, id);
	int ret = sqlite3_get_table(file->db, req, &table, &rows, &cols, &error);
	free(req);
	++rows;
	if(ret != SQLITE_OK) {
		file->error = ret;
		free(error);
		return NULL;
	}

	if(rows < 2) {
		file->error = -102;
		sqlite3_free_table(table);
		return NULL;
	}

	assert(cols == 6);

	struct rlx_spectra *out = malloc(sizeof(*out));
	out->id = id;
	out->circuit = rlx_strdup(table[6]);
	out->fitted = table[7][0] == '1';
	out->project_id = project->id;
	ret = sscanf(table[8], "%lf", &out->freq_lower_limit);
	assert(ret == 1);
	ret = sscanf(table[9], "%lf", &out->freq_upper_limit);
	assert(ret == 1);
	out->date_added  = rlx_str_to_time(table[10]);
	out->date_fitted = rlx_str_to_time(table[11]);

	out->datapoints = rlx_get_datapoints(file, id, &out->length);
	sqlite3_free_table(table);
	return out;
}

struct rlx_spectra** rlx_get_all_spectra(struct rlxfile* file, const struct rlx_project* project)
{
	size_t length;
	int *ids = rlx_get_spectra_ids(file, project, &length);

	if(!ids)
		return NULL;

	struct rlx_spectra **out = malloc(sizeof(*out)*(length+1));
	for(size_t i = 0; i < length; ++i) {
		out[i] = rlx_get_spectra(file, project, i);
	}
	out[length] = NULL;

	free(ids);
	return out;
}

int* rlx_get_spectra_ids(struct rlxfile* file, const struct rlx_project* project, size_t* length)
{
	char **table;
	int rows;
	int cols;
	char *error;
	int ret = sqlite3_get_table(file->db, "SELECT ID FROM Files;", &table, &rows, &cols, &error);
	if(ret != SQLITE_OK) {
		file->error = ret;
		free(error);
		if(length)
			*length = 0;
		return NULL;
	}
	++rows;
	printf("%s: got %dx%d\n", __func__, rows, cols);
	assert(cols == 1);

	if(rows < 2) {
		file->error = -101;
		sqlite3_free_table(table);
		return NULL;
	}

	if(length)
		*length = rows-1;

	int *ids = malloc(sizeof(*ids)*(rows-1));

	for(int i = 1; i < rows; ++i) {
		int ret = sscanf(table[i], "%d", &ids[i-1]);
		assert(ret == 1);
	}

	sqlite3_free_table(table);
	return ids;
}

struct rlx_fitparam** rlx_get_fit_parameters(struct rlxfile* file, const struct rlx_project* project, int id, size_t *length)
{
	(void)project;
	if(length)
		*length = 0;
	char *req = rlx_alloc_printf("SELECT pindex,name,value,error,lowerlimit,upperlimit FROM Fitparameters WHERE file_id=%d", id);
	sqlite3_stmt *ppStmt;
	int ret = sqlite3_prepare_v2(file->db, req, strlen(req), &ppStmt, NULL);
	free(req);
	if(ret != SQLITE_OK) {
		file->error = ret;
		return NULL;
	}

	size_t outSize = 8;
	size_t outIndex = 0;
	struct rlx_fitparam **out = malloc(sizeof(*out)*outSize);

	while((ret = sqlite3_step(ppStmt)) == SQLITE_ROW) {
		assert(sqlite3_column_count(ppStmt) == 6);
		struct rlx_fitparam *param = malloc(sizeof(*param));
		param->p_index = sqlite3_column_int(ppStmt, 0);
		param->spectra_id = id;
		param->name = rlx_strdup((char*)sqlite3_column_text(ppStmt, 1));
		param->value = sqlite3_column_double(ppStmt, 2);
		param->error = sqlite3_column_double(ppStmt, 3);
		param->lower_limit = sqlite3_column_double(ppStmt, 4);
		param->upper_limit = sqlite3_column_double(ppStmt, 5);
		if(outIndex == outSize) {
			outSize *= 2;
			assert((out = realloc(out, sizeof(*out)*outSize)));
		}
		out[outIndex] = param;
		++outIndex;
	}

	if(ret != SQLITE_OK && ret != SQLITE_DONE) {
		free(out);
		file->error = ret;
		return NULL;
	}

	if(outIndex == outSize) {
		outSize += 1;
		assert((out = realloc(out, sizeof(*out)*outSize)));
	}
	out[outIndex] = NULL;

	if(length)
		*length = outIndex;
	file->error = sqlite3_finalize(ppStmt);
	return out;
}

int rlx_get_errnum(const struct rlxfile* file)
{
	return file->error;
}

const char* rlx_get_errnum_str(int errnum)
{
	if(errnum == 0)
		return "Success";
	if(errnum > 0)
		return sqlite3_errstr(errnum);
	if(errnum == -100)
		return "No sutch entry";
	if(errnum == -101)
		return "Project contains no spectra";
	if(errnum == -102)
		return "Tried to load non existing spectra";
	return "Unkown error";
}

