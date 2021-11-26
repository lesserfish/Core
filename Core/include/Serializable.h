#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>
#include <functional>

#define SerializableField(Type, Name)                                       \
    Core::Field<Type> __FIELD__##Name = Core::Field<Type>(this, Name, #Name); \
    Type Name
#define SerializableField_S(Type, Name, Func)                                     \
    Core::Field<Type> __FIELD__##Name = Core::Field<Type>(this, Name, #Name, Func); \
    Type Name

#define SerializableField_SD(Type, Name, FuncA, FuncB)                                    \
    Core::Field<Type> __FIELD__##Name = Core::Field<Type>(this, Name, #Name, FuncA, FuncB); \
    Type Name

namespace Core
{
    template <typename T, bool = std::is_fundamental<T>::value>
    struct Serializer
    {
    };

    template <typename T>
    struct Serializer<T, true>
    {
        static std::string Serialize(T object)
        {
            return std::to_string(object);
        }
    };
    template <typename T>
    struct Serializer<T, false>
    {
        static std::string Serialize(T object)
        {
            return object.Save();
        }
    };

    template <typename T>
    struct Deserializer
    {
        static void Deserialize(std::string input, T &object)
        {
            object.Load(input);
        }
    };
    template <>
    struct Deserializer<int>
    {
        static void Deserialize(std::string input, int &object)
        {
            object = std::stoi(input);
        }
    };
    template <>
    struct Deserializer<float>
    {
        static void Deserialize(std::string input, float &object)
        {
            object = std::stof(input);
        }
    };
    template <>
    struct Deserializer<bool>
    {
        static void Deserialize(std::string input, bool &object)
        {
            object = bool(std::stoi(input));
        }
    };
    template <>
    struct Deserializer<char>
    {
        static void Deserialize(std::string input, char &object)
        {
            object = char(std::stoi(input));
        }
    };

    // TODO: stoi stol stoll stof stod stold (int, long, long long, float, double, long double) * (unsigned, signed) Missing: short
    // Fundamental types : short int, unsigned short int, int, unsigned int, long int, unsigned long int, long long int, unsigned long long int
    // bool, signed char unsigned char, char, float, double, long double, std::string
    // Cool to have: std::vector, std::map, std::list

    class FieldBase
    {
    public:
        virtual std::string GetName() = 0;
        virtual std::string Save() = 0;
        virtual void Load(std::string input) = 0;
        ~FieldBase() {}
    };

    class Serializable
    {
    public:
        void RegisterField(FieldBase *ptr)
        {
            FieldList.push_back(ptr);
        }
        virtual std::string Save()
        {
            nlohmann::json joutput;
            for (auto &f : FieldList)
            {
                joutput[f->GetName()] = f->Save();
            }
            std::string output = joutput.dump();
            return output;
        }
        virtual void Load(std::string input)
        {
            nlohmann::json jinput = nlohmann::json::parse(input);
            for (auto &f : FieldList)
            {
                std::string Content = jinput[f->GetName()];
                f->Load(Content);
            }
        }

    private:
        std::vector<FieldBase *> FieldList;
    };
    template <typename T>
    class Field : public FieldBase
    {
    public:
        Field(Serializable *caller,
              T &_object, std::string _name,
              std::function<std::string(T)> _SerializerFunctor = Serializer<T>::Serialize,
              std::function<void(std::string, T &)> _DeserializerFunctor = Deserializer<T>::Deserialize)
            : object(_object), name(_name), SerializerFunctor(_SerializerFunctor), DeserializerFunctor(_DeserializerFunctor)
        {
            caller->RegisterField(this);
        }
        std::string GetName()
        {
            return name;
        }
        std::string Save()
        {
            return SerializerFunctor(object);
        }
        void Load(std::string input)
        {
            DeserializerFunctor(input, object);
        }

    private:
        std::function<std::string(T)> SerializerFunctor;
        std::function<void(std::string, T &)> DeserializerFunctor;
        T &object;
        const std::string name;
    };
}