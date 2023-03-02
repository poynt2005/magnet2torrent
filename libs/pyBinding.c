#include <Python.h>
#include "GetTorrent.h"

static PyObject *PyCFunc_GetTorrent(PyObject *self, PyObject *args)
{
    const char *magnetURI = NULL;
    if (!PyArg_ParseTuple(args, "s", &magnetURI))
    {
        PyErr_SetString(PyExc_ValueError, "Wrong arguments, expected one string");
        return NULL;
    }
    char *torrentFile = NULL;
    char *torrentFileName = NULL;
    int size = 0;
    if (GetTorrent(magnetURI, &torrentFileName, &torrentFile, &size))
    {
        if (torrentFileName == NULL || torrentFileName == NULL)
        {
            return Py_None;
        }

        PyObject* pyBytesString = PyBytes_FromStringAndSize(torrentFile, size);
        PyObject* pyTorrentInfo = PyDict_New();
        PyDict_SetItemString(pyTorrentInfo, "torrent_name", PyUnicode_FromString(torrentFileName));
        PyDict_SetItemString(pyTorrentInfo, "torrent_binary", pyBytesString);

        ReleaseTorrent(&torrentFileName, &torrentFile);

        return pyTorrentInfo;
    }
    else
    {
        return Py_None;
    }
}

static PyMethodDef methods[] = {
    {"GetTorrent", PyCFunc_GetTorrent, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT,
    "PyGetTorrent",
    NULL,
    -1,
    methods};

PyMODINIT_FUNC
PyInit_PyGetTorrent(void)
{
    return PyModule_Create(&module);
}