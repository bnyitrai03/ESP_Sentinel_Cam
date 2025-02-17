from esp_docs.conf_docs import *  # noqa: F403,F401

extensions += [
    'sphinx_copybutton',
    'esp_docs.esp_extensions.dummy_build_system',
    'esp_docs.esp_extensions.run_doxygen',
]

# Repository and project settings
github_user = 'bnyitrai03'
github_repo = 'ESP_Sentinel_Cam'

# HTML context for theme and GitHub integration
html_context['github_user'] = github_user
html_context['github_repo'] = github_repo
html_context['github_version'] = 'main'

# Project information
project = github_repo
author = 'Levente Nyiri, Bence Nyitrai'
copyright = '2025, ' + author

# Documentation settings
languages = ['en']
language = 'en'
todo_include_todos = True
html_static_path = []

html_baseurl = f'https://bnyitrai03.github.io/{github_repo}/'
