#include "Storage.h"
#include "storage-place.h"
#include <algorithm>
#include <time.h>

Storage::Storage(){
    for(int i = 0; i<10;i++){
        for(int j=0; j<10; j++){
            for(int k=0; k<10; k++){
                int volume = rand()%100 +10;
                if(volume % 5 == 0){
                    storageSpace[i][j][k] = -1;
                }else{
                    storageSpace[i][j][k] = volume;
                }
            }
        }
    }
    for(int i = 0; i<10;i++){
        for(int j=0; j<10; j++){
            for(int k=0; k<10; k++)
                availableSpace[i][j][k] = true;
        }
    }
}

string Storage::getProductId(Product product){
    string id = product.getProductName();
    id+="/";
    id+=product.getExpDate();
    return id;
}

void Storage::findNewPlaceAndAdd(Product product, string id, vector<Product>& products, bool show){
    for(int i = 0;i<10;i++){
        for(int j = 0;j<10;j++){
            for(int k = 0;k<10;k++){
                if(availableSpace[i][j][k]){
                    if(storageSpace[i][j][k] >= product.getQuantity()){
                        product.setStoragePlace(i, j, k);
                        products.push_back(product);
                        availableProducts.insert(pair<string, vector<Product> >(id, products));
                        availableSpace[i][j][k] = false;
                        if(show){
                            system("CLS");
                            cout<<"Your product was successfully stored on place "<<i<<"/"<<j<<"/"<<k<<endl;
                            cout<<"There is "<<storageSpace[i][j][k]-product.getQuantity()<<" "<<product.getUnit()<<" free space in the same number."<<endl;
                            //TODO to file the addition
                        }
                        return ;
                    }
                }
            }
        }
    }
    if(show){
        system("CLS");
        cout<<"Sorry, either the storage is full, or the quantity you are trying to add is too much."<<endl;
    }
}

void Storage::addProduct(Product product, bool show){
    string id = getProductId(product);
    map<string, vector<Product> >::iterator it = availableProducts.find(id);
    vector<Product> products;
    if(it != availableProducts.end())
    {
        cout<<id<<endl;
        products = it->second;
        int filled = 0;
        cout<<products.size()<<endl;
        for (unsigned i=0; i < products.size(); i++) {
            StoragePlace currentPlace = products[i].getStoragePlace();

            double newQuantity = products[i].getQuantity() + product.getQuantity();
            if( newQuantity <= storageSpace[currentPlace.getSection()][currentPlace.getShelf()][currentPlace.getNumber()]){
                products[i].setQuantity(newQuantity);
                if(show){
                    system("CLS");
                    cout<<"The product "<<product.getProductName()<<" was stored on place "<<currentPlace.getSection()<<"/"<<currentPlace.getShelf()<<"/"<<currentPlace.getNumber()<<endl;
                    cout<<"There is "<<storageSpace[currentPlace.getSection()][currentPlace.getShelf()][currentPlace.getNumber()]-newQuantity<<" "<<product.getUnit()<<" free space in the same number."<<endl;
                    ///TODO to file the addition
                }
                return;
            }
        }
        findNewPlaceAndAdd(product, id, products, show);
    }else{
        vector<Product> newVector;
        findNewPlaceAndAdd(product, id, newVector, show);
    }
}

void Storage::listAvailableProducts(){
    bool found = false;
    for(map<string, vector<Product> >::iterator it = availableProducts.begin(); it!=availableProducts.end(); it++ ){
        vector<Product> products = it->second;
        for(unsigned i = 0;i<products.size(); i++ ){
            products[i].output();
            found = true;
        }
    }
    if(!found){
        cout<<"The storage is empty."<<endl;
    }
    system("PAUSE");
}

void Storage::removeProduct(string name, double quantity){
    vector<Product> foundProducts;
    for(map<string, vector<Product> >::iterator it = availableProducts.begin(); it!=availableProducts.end(); it++ ){
        string id = it->first;
        if (id.find(name) != string::npos) {
            foundProducts.insert(foundProducts.end(), it->second.begin(), it->second.end());
            availableProducts.erase(it);
        }
    }
    sortByExpiryDate(foundProducts);
    if(foundProducts.size() == 0){
        cout<<"There isn't available products with this name."<<endl;
        return;
    }
    double availableQuantity = getProductQuantity(foundProducts);
    if( availableQuantity < quantity){
        cout<<"There is only "<<availableQuantity<<" quantity of the selected product. \n For removing it all enter y, for keeping it press n.";
        char command;
        cin>>command;
        while(command!='n'&&command!='y'){
            cin>>command;
        }
        if(command == 'y'){
            cout<<"All products with name: "<<name<<" were removed."<<endl;
        }else{
            for(unsigned i = 0;i < foundProducts.size(); i++){
                addProduct(foundProducts[i], false);
            }
            return;
        }
    }
    for(unsigned i = 0; i<foundProducts.size(); i++){
        if(foundProducts[i].getQuantity()>quantity){
            cout<<"Removing "<<quantity<<" from :"<<endl;
            foundProducts[i].output();
            foundProducts[i].setQuantity(foundProducts[i].getQuantity()- quantity);
            break;
        }
        if(foundProducts[i].getQuantity()<=quantity){
            quantity -= foundProducts[i].getQuantity();
            cout<<"Removing product :"<<endl;
            foundProducts[i].output();
            foundProducts.erase(foundProducts.begin());
            i--;
        }
    }

    for(unsigned i = 0; i<foundProducts.size(); i++){
        addProduct(foundProducts[i], false);
    }

}

struct CompareDates{
    inline bool operator()(Product product1, Product product2)
    {
        return (product1.getExpDate() < product2.getExpDate());
    }
};

void Storage::sortByExpiryDate(vector<Product>& products){
    sort(products.begin(), products.end(), CompareDates());
}

double Storage::getProductQuantity(vector<Product>& products) const{
    double sum = 0;
    for(unsigned int i = 0;i<products.size(); i++){
        sum += products[i].getQuantity();
    }
    return sum;
}
void Storage::clearOutdatedProducts(){
    for(map<string, vector<Product> >::iterator it = availableProducts.begin(); it!=availableProducts.end(); it++ ){
        string id = it->first;
        size_t pos = id.find("/");
        string date = id.substr (pos+1);
        if(date<getCurrentDate()){
            for(unsigned i = 0;i<it->second.size(); i++){
                cout<<"Removing product: "<<endl;
                it->second[i].output();
            }
            availableProducts.erase(it);
        }
    }
}

string Storage::getCurrentDate() const {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y.%m.%d", &tstruct);

    return buf;
}

