/*
 * This file is part of `et engine`
 * Copyright 2009-2013 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#include <et/locale/locale.h>
#include <et/app/application.h>
#include <et/rendering/rendercontext.h>
#include <et/app/pathresolver.h>

using namespace et;

void StandardPathResolver::setRenderContext(RenderContext* rc)
{
	_rc = rc;
	_baseFolder = application().environment().applicationInputDataFolder();
	
	pushSearchPath(application().environment().applicationPath());
	pushSearchPath(_baseFolder);
}

void StandardPathResolver::validateCaches()
{
	ET_ASSERT(_rc != nullptr)
	
	if (locale::currentLocale() != _cachedLocale)
	{
		_cachedLocale = locale::currentLocale();
		
		_cachedLang = "." + locale::localeLanguage(_cachedLocale);
		_cachedSubLang = locale::localeSubLanguage(_cachedLocale);
		
		_cachedLanguage = _cachedLang + "-" + _cachedSubLang;
	}
	
	if (_rc->screenScaleFactor() != _cachedScreenScaleFactor)
	{
		_cachedScreenScaleFactor = _rc->screenScaleFactor();
		
		_cachedScreenScale = (_cachedScreenScaleFactor > 1) ?
			"@" + intToStr(_cachedScreenScaleFactor) + "x" : std::string();
	}
}

std::string StandardPathResolver::resolveFilePath(const std::string& input)
{
	validateCaches();
	
	auto ext = "." + getFileExt(input);
	auto name = removeFileExt(getFileName(input));
	auto path = getFilePath(input);
	
	std::string suggested = input;
	
	auto paths = resolveFolderPaths(path);
	pushSearchPaths(paths);

	for (const auto& folder : _searchPath)
	{
		auto baseName = folder + name;
		
		if (_cachedScreenScaleFactor > 0)
		{
			// path/file@Sx.ln-sb.ext
			suggested = baseName + _cachedScreenScale + _cachedLanguage + ext;
			if (fileExists(suggested))
				break;

			// path/file@Sx.ln.ext
			suggested = baseName + _cachedScreenScale + _cachedLang + ext;
			if (fileExists(suggested))
				break;
		}
		
		// path/file.ln-sb.ext
		suggested = baseName + _cachedLanguage + ext;
		if (fileExists(suggested))
			break;

		// path/file.ln.ext
		suggested = baseName + _cachedLanguage + ext;
		if (fileExists(suggested))
			break;
		
		if (_cachedScreenScaleFactor > 0)
		{
			// path/file@Sx.ext
			suggested = baseName + _cachedScreenScale + ext;
			if (fileExists(suggested))
				break;
		}
		
		// path/file.ext
		suggested = baseName + ext;
		if (fileExists(suggested))
			break;
	}
	
	popSearchPaths(paths.size());
	
	if (!fileExists(suggested))
		log::warning("Unable to resolve file name: %s", input.c_str());
	
	return suggested;
}

std::set<std::string> StandardPathResolver::resolveFolderPaths(const std::string& input)
{
	validateCaches();

	std::set<std::string> result;
	if (input.empty())
		result.insert(_baseFolder);
	
	auto suggested = addTrailingSlash(input + _cachedLanguage);
	if (folderExists(suggested))
		result.insert(suggested);
	
	suggested = addTrailingSlash(input + _cachedLang);
	if (folderExists(suggested))
		result.insert(suggested);
	
	if (folderExists(input))
		result.insert(input);
	
	for (const auto& path : _searchPath)
	{
		auto base = path + input;
		
		suggested = addTrailingSlash(base + _cachedLanguage);
		if (folderExists(suggested))
			result.insert(suggested);
		
		suggested = addTrailingSlash(base + _cachedLang);
		if (folderExists(suggested))
			result.insert(suggested);
		
		suggested = addTrailingSlash(base);
		if (folderExists(suggested))
			result.insert(suggested);
	}
	
	return result;
}

std::string StandardPathResolver::resolveFolderPath(const std::string& input)
{
	validateCaches();
	
	if (input.empty())
		return _baseFolder;
	
	auto suggested = addTrailingSlash(input + _cachedLanguage);
	if (folderExists(suggested))
		return suggested;

	suggested = addTrailingSlash(input + _cachedLang);
	if (folderExists(suggested))
		return suggested;
	
	if (folderExists(input))
		return input;
	
	for (const auto& path : _searchPath)
	{
		suggested = addTrailingSlash(path + input + _cachedLanguage);
		if (folderExists(suggested))
			return suggested;
		
		suggested = addTrailingSlash(path + input + _cachedLang);
		if (folderExists(suggested))
			return suggested;
		
		suggested = addTrailingSlash(path + input);
		if (folderExists(suggested))
			return suggested;
	}
	
	return input;
}

void StandardPathResolver::pushSearchPath(const std::string& path)
{
	_searchPath.push_front(addTrailingSlash(normalizeFilePath(path)));
}

void StandardPathResolver::pushSearchPaths(const std::set<std::string>& paths)
{
	_searchPath.insert(_searchPath.begin(), paths.begin(), paths.end());
}

void StandardPathResolver::pushRelativeSearchPath(const std::string& path)
{
	_searchPath.emplace_front(addTrailingSlash(normalizeFilePath(application().environment().applicationPath() + path)));
}

void StandardPathResolver::popSearchPaths(size_t amount)
{
	for (size_t i = 0; i < amount; ++i)
	{
		if (_searchPath.size() > 1)
			_searchPath.pop_front();
	}
}