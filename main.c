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

#include <stdio.h>
#include <relaxisloader.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	if(argc < 2) {
		printf("Usage %s [FILE]\n", argc == 1 ? argv[0] : "NULL");
		return 1;
	}

	// Open a RelaxIS3 file and check for errors
	const char *error;
	struct rlxfile *file = rlx_open_file(argv[1], &error);
	if(!file) {
		printf("Unable to open %s: %s\n", argv[1], error);
		return 2;
	}

	// Read all projects from the file just opened
	size_t projectCount;
	struct rlx_project** projects = rlx_get_projects(file, &projectCount);
	if(projectCount < 1) {
		printf("File contains no projects: %s\n", rlx_get_errnum_str(rlx_get_errnum(file)));
		return 4;
	}

	// Get the ids of specra associated with the first project in the file
	size_t idCount;
	int *ids = rlx_get_spectra_ids(file, projects[0], &idCount);
	for(int i = 0; i < idCount; ++i)
		printf("PROJECT: %d ID: %d\n", projects[0]->id, ids[i]);
	if(idCount < 1) {
		printf("No spectra in project %d: %s\n", projects[0]->id, rlx_get_errnum_str(rlx_get_errnum(file)));
		return 3;
	}

	printf("%p %d\n", projects[0], projects[0]->id);

	// Grab the first spectrum
	struct rlx_spectra* spectra = rlx_get_spectra(file, projects[0], ids[0]);
	if(!spectra) {
		printf("Could not load spectra for %d %d: %s\n", projects[0]->id, ids[0], rlx_get_errnum_str(rlx_get_errnum(file)));
		return 3;
	}
	printf("Spectra for PROJECT: %d ID: %d\nomega, re, im\n", projects[0]->id, ids[0]);
	for(size_t i = 0; i < spectra->length; ++i) {
		printf("%f,%f,%f\n", spectra->datapoints[i].omega, spectra->datapoints[i].re, spectra->datapoints[i].im);
	}

	size_t paramCount;
	// Grab the parameters for the first spectrum
	struct rlx_fitparam** params = rlx_get_fit_parameters(file, projects[0], ids[0], &paramCount);
	if(!params) {
		printf("Could not get parameters for project %d spectra %d: %s\n", projects[0]->id, ids[0], rlx_get_errnum_str(rlx_get_errnum(file)));
		return 4;
	}
	for(size_t i = 0; i < paramCount; ++i) {
		printf("Parameter %d: Name: %s Value: %f Error: %f\n", params[i]->p_index, params[i]->name, params[i]->value, params[i]->error);
	}

	// Free aquired structs
	rlx_project_free_array(projects);
	rlx_spectra_free(spectra);
	rlx_fitparam_free_array(params);
	free(ids);

	// Close RelaxIS3 file
	rlx_close_file(file);
	return 0;
}
