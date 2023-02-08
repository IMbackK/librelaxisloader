#include <stdio.h>
#include <relaxisloader.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	if(argc < 2) {
		printf("Usage %s [FILE]\n", argc == 1 ? argv[0] : "NULL");
		return 1;
	}

	const char *error;
	struct rxfile *file = rlx_open_file(argv[1], &error);
	if(!file) {
		printf("Unable to open %s: %s\n", argv[1], error);
		return 2;
	}

	size_t projectCount;
	struct rlx_project** projects = rlx_get_projects(file, &projectCount);
	if(projectCount < 1) {
		printf("File contains no projects: %s\n", rlx_get_errnum_str(rlx_get_errnum(file)));
		return 4;
	}

	size_t idCount;
	int *ids = rlx_get_spectra_ids(file, projects[0], &idCount);
	for(int i = 0; i < idCount; ++i)
		printf("PROJECT: %d ID: %d\n", projects[0]->id, ids[i]);
	if(idCount < 1) {
		printf("No spectra in project %d: %s\n", projects[0]->id, rlx_get_errnum_str(rlx_get_errnum(file)));
		return 3;
	}

	printf("%p %d\n", projects[0], projects[0]->id);
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
	struct rlx_fitparam** params = rlx_get_fit_parameters(file, projects[0], ids[0], &paramCount);
	if(!params) {
		printf("Could not get parameters for project %d spectra %d: %s\n", projects[0]->id, ids[0], rlx_get_errnum_str(rlx_get_errnum(file)));
		return 4;
	}
	for(size_t i = 0; i < paramCount; ++i) {
		printf("Parameter %d: Name: %s Value: %f Error: %f\n", params[i]->p_index, params[i]->name, params[i]->value, params[i]->error);
	}

	rlx_project_free_array(projects);
	rlx_spectra_free(spectra);
	rlx_fitparam_free_array(params);
	free(ids);
	rlx_close_file(file);
	return 0;
}
