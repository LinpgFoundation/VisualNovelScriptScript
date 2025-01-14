#ifndef VNS_SCHEMA_HPP
#define VNS_SCHEMA_HPP

#include <nlohmann/json-schema.hpp>

const std::string VNS_SCHEMA = R"(
    {
        "$schema": "http://json-schema.org/draft-07/schema#",
        "properties": {
            "compiler": {
                "properties": {
                    "compiledAt": {
                        "type": "integer"
                    },
                    "patch": {
                        "type": "integer"
                    },
                    "reversion": {
                        "type": "integer"
                    },
                    "version": {
                        "type": "integer"
                    }
                },
                "required": [
                    "compiledAt",
                    "reversion",
                    "version"
                ],
                "type": "object"
            },
            "dialogues": {
                "additionalProperties": {
                    "properties": {
                        "background_image": {
                            "type": "string"
                        },
                        "background_music": {
                            "type": "string"
                        },
                        "character_images": {
                            "items": {
                                "type": "string"
                            },
                            "type": "array"
                        },
                        "comments": {
                            "items": {
                                "type": "string"
                            },
                            "type": "array"
                        },
                        "contents": {
                            "items": {
                                "type": "string"
                            },
                            "type": "array"
                        },
                        "events": {
                            "items": {
                                "properties": {
                                    "target": {
                                        "type": "string"
                                    },
                                    "type": {
                                        "type": "string"
                                    },
                                    "value": {
                                        "type": [
                                            "string",
                                            "number",
                                            "boolean"
                                        ]
                                    }
                                },
                                "required": [
                                    "target",
                                    "type",
                                    "value"
                                ],
                                "type": "object"
                            },
                            "type": "array"
                        },
                        "narrator": {
                            "type": "string"
                        },
                        "next": {
                            "properties": {
                                "target": {
                                    "oneOf": [
                                        {
                                            "type": "string"
                                        },
                                        {
                                            "items": {
                                                "properties": {
                                                    "id": {
                                                        "type": "string"
                                                    },
                                                    "text": {
                                                        "type": "string"
                                                    }
                                                },
                                                "required": [
                                                    "id",
                                                    "text"
                                                ],
                                                "type": "object"
                                            },
                                            "type": "array"
                                        }
                                    ]
                                },
                                "type": {
                                    "type": "string"
                                }
                            },
                            "required": [
                                "type"
                            ],
                            "type": "object"
                        },
                        "previous": {
                            "type": "string"
                        }
                    },
                    "type": "object"
                },
                "type": "object"
            },
            "id": {
                "type": "string"
            },
            "language": {
                "type": "string"
            }
        },
        "required": [
            "compiler",
            "dialogues",
            "id",
            "language"
        ],
        "type": "object"
    }
)";

const nlohmann::json VNS_SCHEMA_JSON = nlohmann::json::parse(VNS_SCHEMA);

#endif
