var LibraryPaths = {
    PlatformJS_GetApplicationPath: function() {
        var path = location.href.substring(0, location.href.lastIndexOf("/"));
         // 'path.length' would return the length of the string as UTF-16 units, but Emscripten C strings operate as UTF-8.
        var lengthBytes = lengthBytesUTF8(path) + 1;
        var buffer = _malloc(lengthBytes);
        Module.stringToUTF8(path, buffer, lengthBytes);
        return buffer;
    }
};
mergeInto(LibraryManager.library, LibraryPaths);
