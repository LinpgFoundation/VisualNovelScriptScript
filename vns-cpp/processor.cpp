#include <string>
#include <vector>
#include "processor.h"
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include "functions.h"
#include <fstream>
#include <sstream>
#include "naming.h"

int Processor::get_id() const
{
	return id_;
}

std::string Processor::get_language() const
{
	return lang_;
}

std::string Processor::ensure_not_null(const std::string& text)
{
	return (iequals(text, "null") || iequals(text, "none")) ? "" : text;
}

std::string Processor::extract_parameter(const std::string& text)
{
	return ensure_not_null(extract_string(text));
}

std::string Processor::extract_tag(const std::string& text)
{
	return text.substr(text.find(TAG_STARTS) + 1, text.find(TAG_ENDS));
}

std::string Processor::extract_string(const std::string& text)
{
	return trim(text.substr(text.find(TAG_ENDS) + 1));
}

[[noreturn]] void Processor::terminated(const std::string& reason) const
{
	throw std::runtime_error("File \"" + path_in_ + "\", line " + std::to_string(line_index_ + 1) +
		"\n  " + get_current_line() + "\nFail to compile due to " + reason);
}

std::string Processor::get_current_line() const
{
	return lines_[line_index_];
}

std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::any>>>
Processor::get_output() const
{
	std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::any>>> output =
		{};
	for (const auto& [section_name, section_contents] : output_)
	{
		output[section_name] = {};
		for (const auto& [dialogue_id, dialogue_content] : section_contents)
		{
			output[section_name][dialogue_id] = dialogue_content.to_map();
		}
	}
	return output;
}


void Processor::process(const std::string& path)
{
	path_in_ = path;
	int current_index = 0;

	if (path_in_.ends_with(SCRIPTS_FILE_EXTENSION))
	{
		std::ifstream file(path_in_);
		std::string line;
		while (std::getline(file, line))
		{
			lines_.emplace_back(line);
		}
	}

	if (lines_.empty())
	{
		terminated("Cannot convert an empty script file!");
	}

	std::string last_label;

	for (size_t index = 0; index < lines_.size(); ++index)
	{
		lines_[index] = trim(lines_[index].substr(0, lines_[index].find(NOTE_PREFIX)));

		if (lines_[index].starts_with(TAG_STARTS))
		{
			if (auto tag = extract_tag(lines_[index]); tag == "label")
			{
				if (!last_label.empty())
				{
					terminated("This label is overwriting the previous one");
				}

				last_label = extract_parameter(lines_[index]);
				if (RESERVED_WORDS.contains(last_label))
				{
					terminated("You cannot use reserved word '" + last_label + "' as a label");
				}
			}
			else if (tag == "section")
			{
				current_index = 0;
			}
		}
		else if (lines_[index].find(":") != std::string::npos)
		{
			dialog_associate_key_[index] = (current_index == 0
				                                ? "head"
				                                : (current_index < 10
					                                   ? "~0" + std::to_string(current_index)
					                                   : "~" + std::to_string(current_index)));

			last_label = "";
			++current_index;
		}
	}

	convert(0);
	lines_.clear();

	// making sure essential instances are init correctly
	if (id_ < 0)
	{
		terminated("You have to set a nonnegative id!");
	}
	if (lang_.empty())
	{
		terminated("You have to set lang!");
	}
	if (section_.empty())
	{
		terminated("You have to set section!");
	}
}

void Processor::convert(int starting_index)
{
	line_index_ = starting_index;

	while (line_index_ < lines_.size())
	{
		std::string currentLine = get_current_line();

		if (currentLine.empty() || lines_[line_index_].find(NOTE_PREFIX) == 0)
		{
			// Skip empty lines or lines starting with #
			// Do nothing
		}
		else if (currentLine.starts_with(COMMENT_PREFIX) == 0)
		{
			// Accumulate comments
			accumulated_comments_.push_back(currentLine.substr(COMMENT_PREFIX.length()));
		}
		else if (currentLine.find(TAG_STARTS) == 0)
		{
			std::string tag = extract_tag(currentLine);

			if (ALTERNATIVES.contains(tag))
			{
				tag = ALTERNATIVES.at(tag);
			}

			if (tag == "bgi")
			{
				current_data_.background_image = extract_parameter(currentLine);
			}
			else if (tag == "bgm")
			{
				current_data_.background_music = extract_parameter(currentLine);
			}
			else if (tag == "show")
			{
				auto names = extract_string(currentLine);
				std::istringstream iss(names);
				std::copy(std::istream_iterator<std::string>(iss),
				          std::istream_iterator<std::string>(),
				          std::back_inserter(current_data_.character_images));
			}
			else if (tag == "hide")
			{
				auto names = extract_string(currentLine);
				if (names == "*")
				{
					current_data_.character_images.clear();
				}
				else
				{
					std::erase_if(current_data_.character_images,
					              [&](const std::string& name)
					              {
						              return Naming(name).Equal(names);
					              });
				}
			}
			else if (tag == "display")
			{
				current_data_.character_images.clear();
				auto names = extract_string(currentLine);
				std::istringstream iss(names);
				std::copy(std::istream_iterator<std::string>(iss),
				          std::istream_iterator<std::string>(),
				          std::back_inserter(current_data_.character_images));
			}
			else if (tag == "id")
			{
				std::string id_str = extract_parameter(currentLine);
				if (!id_str.empty())
				{
					id_ = std::stoi(id_str);
				}
				else
				{
					terminated("Chapter id cannot be None!");
				}
			}
			else if (tag == "language")
			{
				lang_ = extract_string(currentLine);
			}
			else if (tag == "section")
			{
				if (!previous_.empty())
				{
					output_[section_][previous_].next = kNullContentNext;
				}
				section_ = extract_string(currentLine);
				output_[section_] = {};
				output_[section_]["head"] = Content({}, "head");
				current_data_ = Content({}, "head");
				previous_ = "";
			}
			else if (tag == "end")
			{
				assert(!previous_.empty());
				output_[section_][previous_].next = kNullContentNext;
			}
			else if (tag == "scene")
			{
				assert(!previous_.empty());
				output_[section_][previous_].next = ContentNext("scene", {});
				current_data_.background_image = extract_parameter(currentLine);

				if (!current_data_.background_image.empty() && current_data_.background_image.length() == 0)
				{
					current_data_.background_image = nullptr;
				}
				blocked_ = true;
			}
			else if (tag == "block")
			{
				if (!previous_.empty())
				{
					output_[section_][previous_].next = kNullContentNext;
				}
				current_data_ = Content({}, "id_needed");
				previous_ = "";
			}
			else if (tag == "option")
			{
				if (current_data_.contents.empty())
				{
					terminated("Invalid option syntax: '->' cannot be found!");
				}

				if (output_[section_][previous_].next.get_type() != "options")
				{
					output_[section_][previous_].next = ContentNext("options", {});
				}

				auto src_to_target = extract_string(currentLine);

				output_[section_][previous_].next.get_targets().push_back({
					{"text", trim(src_to_target.substr(0, src_to_target.find("->")))},
					{"id", ensure_not_null(trim(src_to_target.substr(src_to_target.find("->") + 2)))}
				});
			}
			// Placeholder, no action needed for "label" tag
			else if (tag == "label")
			{
			}
			else
			{
				terminated("Invalid tag " + tag);
			}
		}
		else if (currentLine.find(':') != std::string::npos)
		{
			std::string narrator = ensure_not_null(currentLine.substr(0, currentLine.size() - 1));
			current_data_.narrator = narrator;

			// Rest of the logic for processing dialog content
			std::vector<std::string> narrator_possible_images;
			std::string narrator_lower_case = toLowerCase(current_data_.narrator);
			if (Naming::GetDatabase().contains(narrator_lower_case))
			{
				narrator_possible_images = Naming::GetDatabase()[narrator_lower_case];
			}
			for (size_t i = 0; i < current_data_.character_images.size(); ++i)
			{
				Naming name_data(current_data_.character_images[i]);
				if (std::find(narrator_possible_images.begin(), narrator_possible_images.end(), name_data.GetName()) !=
					narrator_possible_images.end())
				{
					if (name_data.GetTags().contains("silent"))
					{
						name_data.GetTags().erase("silent");
					}
				}
				else
				{
					name_data.GetTags().insert("silent");
				}
				current_data_.character_images[i] = name_data.ToString();
			}

			current_data_.contents.clear();

			for (size_t sub_index = line_index_ + 1; sub_index < lines_.size(); ++sub_index)
			{
				if (lines_[sub_index].find("- ") == 0)
				{
					current_data_.contents.push_back(trim(lines_[sub_index].substr(2)));
				}
				else
				{
					break;
				}
			}

			if (section_.empty())
			{
				terminated("You have to specify section before script");
			}
			if (!output_.contains(section_))
			{
				output_[section_] = {};
			}

			if (!previous_.empty())
			{
				if (!blocked_)
				{
					current_data_.previous = previous_;
				}
				else
				{
					current_data_.previous = nullptr;
					blocked_ = false;
				}

				if (output_[section_].contains(previous_))
				{
					if (output_[section_][previous_].has_next())
					{
						if (output_[section_][previous_].next.get_type() != "options")
						{
							output_[section_][previous_].next = ContentNext(
								output_[section_][previous_].next.get_type(), dialog_associate_key_[line_index_]);
						}
					}
					else
					{
						output_[section_][previous_].next = ContentNext("default", dialog_associate_key_[line_index_]);
					}
				}
				else
				{
					terminated("KeyError: " + previous_);
				}
			}
			else
			{
				current_data_.previous = nullptr;
			}

			if (!accumulated_comments_.empty())
			{
				current_data_.comments = accumulated_comments_;
				accumulated_comments_.clear();
			}

			previous_ = dialog_associate_key_[line_index_];
			line_index_ += current_data_.contents.size();
			output_[section_][previous_] = Content(current_data_.to_map(), previous_);
			current_data_.comments.clear();
		}
		else
		{
			terminated("Invalid code or content!");
		}
		// Move to the next line
		++line_index_;
	}
}
