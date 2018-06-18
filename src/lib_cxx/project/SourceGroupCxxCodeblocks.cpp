#include "project/SourceGroupCxxCodeblocks.h"

#include "data/indexer/IndexerCommand.h"
#include "settings/ApplicationSettings.h"
#include "settings/SourceGroupSettingsCxxCodeblocks.h"
#include "utility/messaging/type/MessageStatus.h"
#include "utility/codeblocks/CodeblocksProject.h"
#include "utility/utility.h"
#include "Application.h"

SourceGroupCxxCodeblocks::SourceGroupCxxCodeblocks(std::shared_ptr<SourceGroupSettingsCxxCodeblocks> settings)
	: m_settings(settings)
{
}

bool SourceGroupCxxCodeblocks::prepareIndexing()
{
	FilePath codeblocksProjectPath = m_settings->getCodeblocksProjectPathExpandedAndAbsolute();
	if (!codeblocksProjectPath.empty() && !codeblocksProjectPath.exists())
	{
		MessageStatus(L"Can't refresh project").dispatch();

		if (std::shared_ptr<Application> application = Application::getInstance())
		{
			if (application->hasGUI())
			{
				application->handleDialog(
					L"Can't refresh. The referenced Code::Blocks project does not exist anymore: " + codeblocksProjectPath.wstr(),
					{ L"Ok" }
				);
			}
		}
		return false;
	}
	return true;
}

std::set<FilePath> SourceGroupCxxCodeblocks::filterToContainedFilePaths(const std::set<FilePath>& filePaths) const
{
	std::set<FilePath> containedFilePaths;

	const std::set<FilePath> indexedPaths = utility::concat(
		getAllSourceFilePaths(), 
		utility::toSet(m_settings->getIndexedHeaderPathsExpandedAndAbsolute())
	);

	const std::vector<FilePathFilter> excludeFilters = m_settings->getExcludeFiltersExpandedAndAbsolute();

	for (const FilePath& filePath : filePaths)
	{
		bool isInIndexedPaths = false;
		for (const FilePath& indexedPath : indexedPaths)
		{
			if (indexedPath == filePath || indexedPath.contains(filePath))
			{
				isInIndexedPaths = true;
				break;
			}
		}

		if (isInIndexedPaths)
		{
			for (const FilePathFilter& excludeFilter : excludeFilters)
			{
				if (excludeFilter.isMatching(filePath))
				{
					isInIndexedPaths = false;
					break;
				}
			}
		}

		if (isInIndexedPaths)
		{
			containedFilePaths.insert(filePath);
		}
	}

	return containedFilePaths;
}

std::set<FilePath> SourceGroupCxxCodeblocks::getAllSourceFilePaths() const
{
	std::set<FilePath> sourceFilePaths;
	if (std::shared_ptr<Codeblocks::Project> project = Codeblocks::Project::load(
		m_settings->getCodeblocksProjectPathExpandedAndAbsolute()
	))
	{
		const std::vector<FilePathFilter> excludeFilters = m_settings->getExcludeFiltersExpandedAndAbsolute();

		for (const FilePath& filePath : project->getAllSourceFilePathsCanonical(m_settings->getSourceExtensions()))
		{
			bool isExcluded = false;
			for (const FilePathFilter& excludeFilter : excludeFilters)
			{
				if (excludeFilter.isMatching(filePath))
				{
					isExcluded = true;
					break;
				}
			}

			if (!isExcluded && filePath.exists())
			{
				sourceFilePaths.insert(filePath);
			}
		}
	}
	return sourceFilePaths;
}

std::vector<std::shared_ptr<IndexerCommand>> SourceGroupCxxCodeblocks::getIndexerCommands(const std::set<FilePath>& filesToIndex) const
{
	std::vector<std::shared_ptr<IndexerCommand>> indexerCommands;
	if (std::shared_ptr<Codeblocks::Project> project = Codeblocks::Project::load(
		m_settings->getCodeblocksProjectPathExpandedAndAbsolute()
	))
	{
		for (std::shared_ptr<IndexerCommand> indexerCommand: project->getIndexerCommands(m_settings, ApplicationSettings::getInstance()))
		{
			if (filesToIndex.find(indexerCommand->getSourceFilePath()) != filesToIndex.end())
			{
				indexerCommands.push_back(indexerCommand);
			}
		}
	}
	return indexerCommands;
}

std::shared_ptr<SourceGroupSettings> SourceGroupCxxCodeblocks::getSourceGroupSettings()
{
	return m_settings;
}

std::shared_ptr<const SourceGroupSettings> SourceGroupCxxCodeblocks::getSourceGroupSettings() const
{
	return m_settings;
}